#version 330

layout(location = 0) in vec3 aPos;

uniform mat4 viewMatrix;
uniform mat4 projectMatrix;

void main() {
    gl_Position = vec4(aPos, 1.0);
}