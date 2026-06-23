#version 450

layout(location = 0) in vec2 iFragUV;

uniform float uHighlightBrightness = 1.0;

layout(binding = 24) uniform sampler2D tScreenMap;

out vec4 oFragColor;

void main() {
    const vec3 luminance = vec3(0.2126, 0.7152, 0.0722); // human eye luminance weight
    vec3 color = texture(tScreenMap, iFragUV).rgb;
    float brightness = dot(color, luminance);
    
    // if brightness is high, keep the color, otherwise set to black
    // so that later can use bloom blur to enhance the bloom effect
    oFragColor = vec4(brightness > uHighlightBrightness ? color : vec3(0.0), 1.0);
}