#version 450

layout(location = 0) in vec2 iFragUV;

layout(binding = 24) uniform sampler2D tScreenMap;
layout(binding = 27) uniform sampler2D tBloomBlurMap;

uniform float uBloomIntensity;

out vec4 oFragColor;

vec3 ACESFilm(vec3 x) {
    float a = 2.51; 
    float b = 0.03; 
    float c = 2.43; 
    float d = 0.59; 
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 hdrColor = texture(tScreenMap, iFragUV).rgb;
    
    // Bloom blur
    vec3 bloomColor = texture(tBloomBlurMap, iFragUV).rgb;
    hdrColor = hdrColor + bloomColor * uBloomIntensity;
    
    // Tone mapping(convert hdr color into sdr color)
    vec3 sdrColor = ACESFilm(hdrColor);

    // Gamma correction
    vec3 finalColor = pow(sdrColor, vec3(1.0 / 2.2));

    oFragColor = vec4(finalColor, 1.0);
}