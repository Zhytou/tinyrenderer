#version 450

layout(location = 0) in vec2 iFragUV;

uniform bool uXFilter;
uniform float uTexelSizeX;
uniform float uTexelSizeY;
layout(binding = 29) uniform sampler2D tBlurXMap;
layout(binding = 30) uniform sampler2D tBlurYMap;

out vec4 oFragColor;

void main() {
    const float weight[5] = float[](0.0625, 0.25, 0.375, 0.25, 0.0625); // gaussian filter weights

    oFragColor = vec4(0.0, 0.0, 0.0, 1.0);
    if (uXFilter) {
        for (int i = -2; i <= 2; i++) {
            oFragColor.rgb += texture(tBlurYMap, iFragUV + vec2(i * uTexelSizeX, 0.0)).rgb * weight[i + 2];
        }
    } else {
        for (int i = -2; i <= 2; i++) {
            oFragColor.rgb += texture(tBlurXMap, iFragUV + vec2(0.0, i * uTexelSizeY)).rgb * weight[i + 2];
        }
    }
}