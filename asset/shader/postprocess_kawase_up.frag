#version 450

layout(location = 0) in vec2 iFragUV;
layout(location = 1) in vec4 iFragUV01;
layout(location = 2) in vec4 iFragUV23;
layout(location = 3) in vec4 iFragUV45;
layout(location = 4) in vec4 iFragUV67;

uniform int uSrcLevel; // source miplevel
uniform int uDstLevel; // destination miplevel

layout(binding = 26) uniform sampler2D tBlurDownMap; // blur down-sampling map
layout(binding = 27) uniform sampler2D tBlurUpMap; // blur up-sampling map

out vec4 oFragColor;

void main() {
    vec4 color = vec4(0.0);
    color += textureLod(tBlurUpMap, iFragUV01.xy, uSrcLevel) * 2.0;
    color += textureLod(tBlurUpMap, iFragUV01.zw, uSrcLevel) * 2.0;
    color += textureLod(tBlurUpMap, iFragUV23.xy, uSrcLevel) * 2.0;
    color += textureLod(tBlurUpMap, iFragUV23.zw, uSrcLevel) * 2.0;
    color += textureLod(tBlurUpMap, iFragUV45.xy, uSrcLevel);
    color += textureLod(tBlurUpMap, iFragUV45.zw, uSrcLevel);
    color += textureLod(tBlurUpMap, iFragUV67.xy, uSrcLevel);
    color += textureLod(tBlurUpMap, iFragUV67.zw, uSrcLevel);
    vec4 upColor = color * 0.0833333; // 1.0 / 12.0
    vec4 downColor = textureLod(tBlurDownMap, iFragUV, uDstLevel);

    oFragColor = vec4(upColor.rgb * 0.3 + downColor.rgb * 0.7, 1.0);
}
