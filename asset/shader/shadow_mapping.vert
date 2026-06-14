#version 450

layout(location = 0) in vec3 iVertPos;

layout(std140, binding = 1) uniform ModelBlock {
    mat4 uModelMatrix;
    mat4 uNormalMatrix;
};
uniform mat4 uLightViewProjMatrix;

void main() {
    gl_Position = uLightViewProjMatrix * uModelMatrix * vec4(iVertPos, 1.0);
}