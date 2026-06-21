#version 450

layout(location = 0) in vec2 iFragUV;
layout(location = 1) in vec4 iFragUV01;
layout(location = 2) in vec4 iFragUV23;

uniform int uSrcLevel; // source miplevel
layout(binding = 26) uniform sampler2D tBlurDownMap; // blur down-sampling map

out vec4 oFragColor;

void main() {
    vec4 color = vec4(0.0);
    color += textureLod(tBlurDownMap, iFragUV, uSrcLevel) * 4.0;
    color += textureLod(tBlurDownMap, iFragUV01.xy, uSrcLevel);
    color += textureLod(tBlurDownMap, iFragUV01.zw, uSrcLevel);
    color += textureLod(tBlurDownMap, iFragUV23.xy, uSrcLevel);
    color += textureLod(tBlurDownMap, iFragUV23.zw, uSrcLevel);
    oFragColor = color * 0.125;  // 1.0 / 8.0
}