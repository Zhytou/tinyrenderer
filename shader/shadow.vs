#version 330

layout(location = 0) in vec3 aVertexPos;

uniform mat4 uLightSpaceMatrix;

void main()
{
    gl_Position = uLightSpaceMatrix * vec4(aVertexPos, 1.0);
}