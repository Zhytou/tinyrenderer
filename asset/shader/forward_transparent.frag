#version 450

#include "common_brdf.glsl"
#include "common_normal.glsl"
#include "common_shadow.glsl"
#include "common_sampling.glsl"
#include "common_raymarch.glsl"

layout(location = 0) in vec3 iFragPos;
layout(location = 1) in vec3 iFragNormal;
layout(location = 2) in vec3 iFragTangent;
layout(location = 3) in vec2 iFragUV;
layout(location = 4) in vec3 iFragView; // view direction from vertex to camera
layout(location = 5) in vec3 iFragScreenUVDepth; // screen space uv and depth

// ubo block
layout(std140, binding = 0) uniform CameraBlock {
    mat4 uViewMatrix;
    mat4 uProjMatrix;
    mat4 uInvViewMatrix;
    mat4 uInvProjMatrix;
    vec3 uCameraPos;
    float uCameraType;
    float uFov;
    float uNear;
    float uFar;
    float uAspect;
};
// ssbo array
struct Light {
    mat4 viewProjMatrix; 
    vec4 colorIntensity;
    vec4 vectorType; // use .w to distinguish between directional and point light
    vec4 uvOffsetScale;
};
layout(std430, binding = 0) buffer LightBuffer {
    Light uLights[];
};
uniform int uLightCount;

layout(binding = 0) uniform sampler2D tAlbedoMap;
layout(binding = 1) uniform sampler2D tNormalMap;
layout(binding = 2) uniform sampler2D tMRAOMap;
layout(binding = 14) uniform samplerCube tIBLDiffuseMap;
layout(binding = 15) uniform samplerCube tIBLSpecularMap;
layout(binding = 16) uniform sampler2D tIBLBRDFLUTMap;
layout(binding = 19) uniform sampler2D tShadowDepthMap; // GL_DEPTH_COMPONENT24, .x is the depth value;
layout(binding = 20) uniform sampler2D tScreenColorMap;
layout(binding = 23) uniform sampler2D tScreenDepthMap;

uniform float uDistortion = 1.0;

out vec4 oFragColor;

void main() {
    vec3 albedo = texture(tAlbedoMap, iFragUV).rgb;
    vec3 mrao   = texture(tMRAOMap, iFragUV).rgb;
    float metallic  = mrao.r;
    float roughness = mrao.g;
    float ao        = mrao.b;

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 V = normalize(iFragView); // frag -> camera
    vec3 N = normalize(iFragNormal); // primitive normal in world space
    vec3 T = normalize(iFragTangent);
    vec3 TN = N_decode(texture(tNormalMap, iFragUV).xyz); // frag normal in tangent space
    N = N_toWorld(N, T, TN);  // convert frag normal to world space with help of primitive normal and frag tangent
    float NdotV = clamp(dot(N, V), 0.0, 1.0);

    // ----------------------------------------------------------------
    // Evaluate direct light reflection color(both diffuse and specular)
    // ----------------------------------------------------------------
    vec3 dReflectionColor = vec3(0.0);
    for (int i = 0; i < uLightCount; i++) {
        vec3 L = uLights[i].vectorType.w == 0.0 ? normalize(-uLights[i].vectorType.xyz) : normalize(uLights[i].vectorType.xyz - iFragPos); // frag -> light
        vec3 lightSpaceUVD = Pos_toLightSpaceUVD(uLights[i].viewProjMatrix, iFragPos);
        vec2 atlasUV = uLights[i].uvOffsetScale.xy + lightSpaceUVD.xy * uLights[i].uvOffsetScale.zw;

        vec3 color = BRDF(L, V, N, F0, albedo, metallic, roughness) * uLights[i].colorIntensity.rgb * uLights[i].colorIntensity.w;
        float visibility = SM(tShadowDepthMap, atlasUV, lightSpaceUVD.z);
        
        dReflectionColor += color * visibility;
    } 

    // ----------------------------------------------------------------
    // Evaluate indirect light reflection color
    // ----------------------------------------------------------------
    vec3 indReflectionColor = vec3(0.0);


    // ----------------------------------------------------------------
    // Evaluate both direct and indirect light refraction color
    // ----------------------------------------------------------------
    float eta = 1 / 1.5;
    vec3 H = GGXSample(iFragUV, N, roughness);
    vec3 R = refract(-V, normalize(H), eta);
    vec4 hit = RayMarch(iFragPos, R, uViewMatrix, uProjMatrix, tScreenDepthMap, uNear, uFar, uCameraType);
    vec3 absorption = vec3(0.1, 0.02, 0.15);
    absorption = exp(-absorption * hit.z);
    vec3 refractionColor = texture(tScreenColorMap, hit.xy).rgb * absorption.rgb * hit.w;
    
    // ----------------------------------------------------------------
    // Evaluate final color
    // ----------------------------------------------------------------
    vec3 F = F_Schlick(NdotV, F0);
    oFragColor = vec4( mix(refractionColor, dReflectionColor + indReflectionColor, F), 1.0);
}

