#version 330

layout(location = 0) in vec3 aVertexPos;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec3 aVertexTangent;
layout(location = 3) in vec2 aVertexUV;

uniform mat4 uViewMatrix;
uniform mat4 uProjectMatrix;
uniform mat4 uLightSpaceMatrix;

out vec3 vFragPos;
out vec3 vFragNormal;
out vec3 vFragTangent;
out vec2 vFragUV;
out vec4 vLightSpaceFragPos;

void main() {
    vFragPos = aVertexPos;
    vFragNormal = aVertexNormal;
    vFragTangent = aVertexTangent;
    vFragUV = aVertexUV;
    vLightSpaceFragPos = uLightSpaceMatrix * vec4(aVertexPos, 1.0);

    gl_Position = uProjectMatrix * uViewMatrix * vec4(aVertexPos, 1.0);
}