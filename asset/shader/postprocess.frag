#version 450

layout(location = 0) in vec2 iFragUV;

uniform float uBloomIntensity = 1.0;
uniform float uFlareIntensity = 0.5;

layout(binding = 24) uniform sampler2D tScreenMap;
layout(binding = 25) uniform sampler2D tHighlightMap; // debug check highlight distribution
layout(binding = 26) uniform sampler2D tBloomblurDownMap; // debug check downsampling result
layout(binding = 27) uniform sampler2D tBloomblurMap;
layout(binding = 28) uniform sampler2D tLensflareMap;
layout(binding = 31) uniform sampler2D tDirtmaskMap;

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
    vec3 bloomColor = texture(tBloomblurMap, iFragUV).rgb;
    hdrColor = hdrColor + bloomColor * uBloomIntensity;
    
    // Lens flare
    vec3 flareColor = texture(tLensflareMap, iFragUV).rgb;
    vec3 dirtColor = texture(tDirtmaskMap, iFragUV).rgb;
    hdrColor = hdrColor + flareColor * dirtColor * uFlareIntensity;
    
    // Tone mapping(convert hdr color into sdr color)
    vec3 sdrColor = ACESFilm(hdrColor);

    // Gamma correction
    vec3 finalColor = pow(sdrColor, vec3(1.0 / 2.2));

    oFragColor = vec4(finalColor, 1.0);
}