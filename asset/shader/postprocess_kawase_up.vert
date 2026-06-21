#version 450

layout(location = 0) in vec2 iVertPos;
layout(location = 1) in vec2 iVertUV;

uniform float uSrcTexelSizeX; // source texture(previous level) texel width
uniform float uSrcTexelSizeY; // source texture(previous level) texel height

layout(location = 0) out vec2 oFragUV;
layout(location = 1) out vec4 oFragUV01;    
layout(location = 2) out vec4 oFragUV23;
layout(location = 3) out vec4 oFragUV45;
layout(location = 4) out vec4 oFragUV67;

void main() {
    float sizeX = uSrcTexelSizeX;
    float sizeY = uSrcTexelSizeY;
    float halfSizeX = uSrcTexelSizeX * 0.5;
    float halfSizeY = uSrcTexelSizeY * 0.5;

    oFragUV = iVertUV;
    oFragUV01 = vec4(iVertUV + vec2(-halfSizeX, -halfSizeY), iVertUV + vec2(-halfSizeX, halfSizeY));
    oFragUV23 = vec4(iVertUV + vec2(halfSizeX, -halfSizeY), iVertUV + vec2(halfSizeX, halfSizeY));
    oFragUV45 = vec4(iVertUV + vec2(-sizeX, 0.0), iVertUV + vec2(sizeX, 0));
    oFragUV67 = vec4(iVertUV + vec2(0.0, -sizeY), iVertUV + vec2(0.0, sizeY));
    gl_Position = vec4(iVertPos, 0.0, 1.0);
}