#version 450

#include "common_brdf.glsl"
#include "common_normal.glsl"
#include "common_shadow.glsl"

layout(location = 0) in vec2 iFragUV;

// ubo block
layout(std140, binding = 0) uniform CameraBlock {
    mat4 uViewMatrix;
    mat4 uProjMatrix;
    mat4 uInvViewProjMatrix;
    vec3 uCameraPos;
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

layout(binding = 8) uniform sampler2D tAlbedoMap;
layout(binding = 9) uniform sampler2D tNormalMap;
layout(binding = 10) uniform sampler2D tMRAOMap;
layout(binding = 11) uniform sampler2D tDepthMap; // GL_DEPTH_COMPONENT24, .x is the depth value
layout(binding = 14) uniform samplerCube tIBLDiffuseMap;
layout(binding = 15) uniform samplerCube tIBLSpecularMap;
layout(binding = 26) uniform samplerCube tIBLBRDFLUTMap;
layout(binding = 19) uniform sampler2D tShadowDepthMap; // GL_DEPTH_COMPONENT24, .x is the depth value

layout(location = 0) out vec4 oFragColor;
// layout(location = 1) out vec3 oFragNormal;
// layout(location = 2) out vec3 oFragMetallicRoughness; 

vec3 Pos_toWord(vec3 pos) {
    vec4 worldPos = uInvViewProjMatrix * vec4(pos, 1.0);
    return worldPos.xyz / worldPos.w;
};

void main() {    
    vec3 albedo = texture(tAlbedoMap, iFragUV).rgb;
    vec3 mrao   = texture(tMRAOMap, iFragUV).rgb;
    float depth = texture(tDepthMap, iFragUV).x;

    float metallic = mrao.r;
    float roughness = mrao.g;
    float ao        = mrao.b;

    vec3 pos = vec3(iFragUV, depth) * 2.0 - 1.0;
    vec3 worldPos = Pos_toWord(pos);

    vec3 V = normalize(uCameraPos - worldPos); // frag -> camera
    vec3 N = N_decode(texture(tNormalMap, iFragUV).xyz);
    vec3 F0 =  mix(vec3(0.04), albedo, metallic);
    
    // ----------------------------------------------------------------
    // Evaluate direct light color
    // ----------------------------------------------------------------
    vec3 dLightColor = vec3(0.0);
    for (int i = 0; i < uLightCount; i++) {
        vec3 L = uLights[i].vectorType.w == 0.0 ? normalize(-uLights[i].vectorType.xyz) : normalize(uLights[i].vectorType.xyz - worldPos); // frag -> light
        vec3 lightSpaceUVD = Pos_toLightSpaceUVD(uLights[i].viewProjMatrix, worldPos);
        vec2 atlasUV = uLights[i].uvOffsetScale.xy + lightSpaceUVD.xy * uLights[i].uvOffsetScale.zw;

        vec3 color = BRDF(L, V, N, F0, albedo, metallic, roughness) * uLights[i].colorIntensity.rgb * uLights[i].colorIntensity.w;
        float visibility = SM(tShadowDepthMap, atlasUV, lightSpaceUVD.z);

        dLightColor += color * visibility;
    }

    // ----------------------------------------------------------------
    // Evaluate indirect light color
    // ----------------------------------------------------------------
    float NdotV = clamp(dot(N, V), 0.0, 1.0);
    vec3 kS = F_Schlick(NdotV, F0);
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 irradiance = texture(tIBLDiffuseMap, N).rgb;
    vec3 indLightColor = kD * albedo * irradiance;

    oFragColor = vec4(dLightColor + indLightColor, 1.0);
}
