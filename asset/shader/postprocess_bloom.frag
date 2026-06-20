#version 450

layout(location = 0) in vec2 iFragUV;

uniform bool uXFilter;
uniform float uTexelSizeX;
uniform float uTexelSizeY;
layout(binding = 26) uniform sampler2D tBloomBlurXMap;
layout(binding = 27) uniform sampler2D tBloomBlurYMap;

out vec4 oFragColor;

void main() {
    const float weight[5] = float[](0.0625, 0.25, 0.375, 0.25, 0.0625);

    oFragColor = vec4(0.0, 0.0, 0.0, 1.0);
    if (uXFilter) {
        for (int i = -2; i <= 2; i++) {
            oFragColor.rgb += texture(tBloomBlurYMap, iFragUV + vec2(i * uTexelSizeX, 0.0)).rgb * weight[i + 2];
        }
    } else {
        for (int i = -2; i <= 2; i++) {
            oFragColor.rgb += texture(tBloomBlurXMap, iFragUV + vec2(0.0, i * uTexelSizeY)).rgb * weight[i + 2];
        }
    }
}