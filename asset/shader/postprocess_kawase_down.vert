#version 450

layout(location = 0) in vec2 iVertPos;
layout(location = 1) in vec2 iVertUV;

uniform float uSrcTexelSizeX; // source texture(previous level) texel width
uniform float uSrcTexelSizeY; // source texture(previous level) texel height

layout(location = 0) out vec2 oFragUV;      // centeral uv
layout(location = 1) out vec4 oFragUV01;    // top-left and bottom-left uv
layout(location = 2) out vec4 oFragUV23;    // top-right and bottom-right uv

void main() {
    oFragUV = iVertUV;
    float halfSizeX = uSrcTexelSizeX * 0.5;
    float halfSizeY = uSrcTexelSizeY * 0.5;
    oFragUV01 = vec4(iVertUV + vec2(-halfSizeX, -halfSizeY), iVertUV + vec2(-halfSizeX, halfSizeY));
    oFragUV23 = vec4(iVertUV + vec2(halfSizeX, -halfSizeY), iVertUV + vec2(halfSizeX, halfSizeY));
    gl_Position = vec4(iVertPos, 0.0, 1.0);
}