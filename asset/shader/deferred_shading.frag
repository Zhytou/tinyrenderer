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
};
layout(std430, binding = 0) buffer LightBuffer {
    Light uLights[];
};
uniform int uLightCount;

layout(binding = 3) uniform sampler2D tAlbedoMap;
layout(binding = 4) uniform sampler2D tNormalMap;
layout(binding = 5) uniform sampler2D tMRAOMap;
layout(binding = 6) uniform sampler2D tDepthMap; // GL_DEPTH_COMPONENT24, .x is the depth value
layout(binding = 7) uniform sampler2D tShadowDepthMap; // GL_DEPTH_COMPONENT24, .x is the depth value

out vec4 oFragColor;

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
    
    oFragColor = vec4(0.0, 0.0, 0.0, 1.0);
    for (int i = 0; i < uLightCount; i++) {
        vec3 lightSpaceUVD = Pos_toLightSpaceUVD(uLights[i].viewProjMatrix, worldPos);
        vec3 L = uLights[i].vectorType.w == 0.0 ? normalize(-uLights[i].vectorType.xyz) : normalize(uLights[i].vectorType.xyz - worldPos); // frag -> light

        vec3 color = BRDF(L, V, N, F0, albedo, metallic, roughness) * uLights[i].colorIntensity.rgb * uLights[i].colorIntensity.w;
        float visibility = SM(tShadowDepthMap, lightSpaceUVD.xy, lightSpaceUVD.z);

        oFragColor.rgb += color;
    }
}
