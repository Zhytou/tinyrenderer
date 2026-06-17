#version 450

layout(location = 0) in vec3 iVertPos;

layout(location = 0) out vec3 oFragDir;

uniform mat4 uViewProjMatrix;

void main() {
    oFragDir = normalize(iVertPos);  
    gl_Position = uViewProjMatrix * vec4(iVertPos, 1.0);
}