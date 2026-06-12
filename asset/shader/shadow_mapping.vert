#version 450

layout(location = 0) in vec3 iVertPos;

layout(std140, binding = 0) uniform LightBlock {
    mat4 uLightSpaceMatrix; 
    vec3 uLightDirection;
    vec3 uLightColor;
};
layout(std140, binding = 2) uniform ModelBlock {
    mat4 uModelMatrix;
    mat4 uNormalMatrix;
};

void main() {
    gl_Position = uLightSpaceMatrix * uModelMatrix * vec4(iVertPos, 1.0);
}