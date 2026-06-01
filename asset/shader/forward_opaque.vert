#version 450

layout(location = 0) in vec3 iVertexPos;
layout(location = 1) in vec3 iVertexNormal;
layout(location = 2) in vec3 iVertexTangent;
layout(location = 3) in vec2 iVertexUV;

uniform mat4 uModelMatrix;
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

layout(location = 0) out vec3 oFragPos;
layout(location = 1) out vec3 oFragNormal;
layout(location = 2) out vec3 oFragTangent;
layout(location = 3) out vec2 oFragUV;
layout(location = 4) out vec4 oLightSpaceFragPos;

void main() {
    vFragPos = (uModelMatrix * vec4(aVertexPos, 1.0)).xyz;
    vFragNormal = (uModelMatrix * vec4(aVertexNormal, 0.0)).xyz;
    vFragTangent = (uModelMatrix * vec4(aVertexTangent, 0.0)).xyz;
    vFragUV = aVertexUV;
    vLightSpaceFragPos = uLightSpaceMatrix * uModelMatrix * vec4(aVertexPos, 1.0);

    gl_Position = uProjectMatrix * uViewMatrix * uModelMatrix * vec4(aVertexPos, 1.0);
}