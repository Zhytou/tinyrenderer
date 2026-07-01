#version 450

#include "common_brdf.glsl"
#include "common_normal.glsl"
#include "common_shadow.glsl"

layout(location = 0) in vec3 iFragPos;
layout(location = 1) in vec3 iFragNormal;
layout(location = 2) in vec3 iFragTangent;
layout(location = 3) in vec2 iFragUV;
layout(location = 4) in vec3 iFragView;

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

layout(location = 0) out vec4 oFragColor;
// layout(location = 1) out vec3 oFragNormal;
// layout(location = 2) out vec3 oFragMetallicRoughness;

void main() {
    vec3 albedo = texture(tAlbedoMap, iFragUV).rgb;
    vec3 mrao   = texture(tMRAOMap, iFragUV).rgb;
    float metallic = mrao.r;
    float roughness = mrao.g;
    float ao        = mrao.b;

    vec3 F0 =  mix(vec3(0.04), albedo, metallic);
    vec3 V = normalize(iFragView); // frag -> camera
    vec3 N = normalize(iFragNormal);
    vec3 T = normalize(iFragTangent);
    vec3 TN = N_decode(texture(tNormalMap, iFragUV).xyz);
    N = N_toWorld(N, T, TN);
    float NdotV = clamp(dot(N, V), 0.0, 1.0);

    // ----------------------------------------------------------------
    // Evaluate direct light color
    // ----------------------------------------------------------------
    vec3 dLightColor = vec3(0.0);
    for (int i = 0; i < uLightCount; i++) {
        vec3 L = uLights[i].vectorType.w == 0.0 ? normalize(-uLights[i].vectorType.xyz) : normalize(uLights[i].vectorType.xyz - iFragPos); // frag -> light
        vec3 lightSpaceUVD = Pos_toLightSpaceUVD(uLights[i].viewProjMatrix, iFragPos);
        vec2 atlasUV = uLights[i].uvOffsetScale.xy + lightSpaceUVD.xy * uLights[i].uvOffsetScale.zw;

        vec3 color = BRDF(L, V, N, F0, albedo, metallic, roughness) * uLights[i].colorIntensity.rgb * uLights[i].colorIntensity.w;
        float visibility = SM(tShadowDepthMap, atlasUV, lightSpaceUVD.z);
        
        dLightColor += color * visibility;
    }    

    // ----------------------------------------------------------------
    // Evaluate indirect light color
    // ----------------------------------------------------------------
    vec3 irradiance = texture(tIBLDiffuseMap, N).rgb;
    vec3 filteredColor = textureLod(tIBLSpecularMap, N, roughness * 5).rgb;
    vec2 brdf = texture(tIBLBRDFLUTMap, vec2(NdotV, roughness)).rg;
    vec3 kS = F0 * brdf.x + brdf.y;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 indLightColor = kD * albedo * irradiance + kS * filteredColor;

    oFragColor = vec4(dLightColor + indLightColor, 1.0);
}