#version 450

#include "common_brdf.glsl"

layout(location = 0) in vec3 iFragPos;
layout(location = 1) in vec3 iFragNormal;
layout(location = 2) in vec3 iFragTangent;
layout(location = 3) in vec2 iFragUV;
layout(location = 4) in vec4 iLightSpaceFragPos;

layout(std140, binding = 0) uniform LightBlock {
    mat4 uLightSpaceMatrix; 
    vec4 uLightColorIntensity;
    vec4 uLightVectorType; // use .w to distinguish between directional and point light
};
layout(std140, binding = 1) uniform CameraBlock {
    mat4 uViewMatrix;
    mat4 uProjectMatrix;
    vec3 uCameraPos;
};

out vec4 oFragColor;

void main() {
    vec3 L = uLightVectorType.xyz;
    vec3 V = normalize(uCameraPos -iFragPos);
    vec3 N = normalize(iFragNormal);

    
    oFragColor = vec4(1.0, 1.0, 1.0, 1.0);
}