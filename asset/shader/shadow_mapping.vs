#version 450

layout(location = 0) in vec3 aVertexPos;

uniform mat4 uModelMatrix;
layout(std140, binding = 0) uniform LightBlock {
    mat4 uLightSpaceMatrix; 
    vec3 uLightDirection;
    vec3 uLightColor;
};

void main()
{
    gl_Position = uLightSpaceMatrix * uModelMatrix * vec4(aVertexPos, 1.0);
}