#version 330

layout(location = 0) in vec3 aVertexPos;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec3 aTexCoords;

uniform mat4 viewMatrix;
uniform mat4 projectMatrix;

out vec3 vFragPos;
out vec3 vFragNormal;

void main() {
    vFragPos = aVertexPos;
    vFragNormal = aVertexNormal;
    
    gl_Position = uProjectMatrix * uViewMatrix * vec4(aVertexPos, 1.0);
}