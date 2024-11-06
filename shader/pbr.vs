#version 330

layout(location = 0) in vec3 aVertexPos;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec3 aVertexTangent;
layout(location = 3) in vec2 aVertexUV;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectMatrix;
uniform mat4 uLightSpaceMatrix;

out vec3 vFragPos;
out vec3 vFragNormal;
out vec3 vFragTangent;
out vec2 vFragUV;
out vec4 vLightSpaceFragPos;

void main() {
    vFragPos = (uModelMatrix * vec4(aVertexPos, 1.0)).xyz;
    vFragNormal = (uModelMatrix * vec4(aVertexNormal, 0.0)).xyz;
    vFragTangent = (uModelMatrix * vec4(aVertexTangent, 0.0)).xyz;
    vFragUV = aVertexUV;
    vLightSpaceFragPos = uLightSpaceMatrix * vec4(aVertexPos, 1.0);

    gl_Position = uProjectMatrix * uViewMatrix * uModelMatrix * vec4(aVertexPos, 1.0);
}