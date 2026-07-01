#version 450

layout(location = 0) in vec2 iFragUV;

uniform float uBloomIntensity = 1.0;
uniform float uFlareIntensity = 0.5;

layout(binding = 20) uniform sampler2D tScreenColorMap;
layout(binding = 25) uniform sampler2D tHighlightMap; // debug check highlight distribution
layout(binding = 26) uniform sampler2D tBloomblurDownMap; // debug check downsampling result
layout(binding = 27) uniform sampler2D tBloomblurMap;
layout(binding = 28) uniform sampler2D tLensflareMap;
layout(binding = 31) uniform sampler2D tDirtmaskMap;

out vec4 oFragColor;

vec3 ACESFilm(vec3 color) {
    float a = 2.51; 
    float b = 0.03; 
    float c = 2.43; 
    float d = 0.59; 
    float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

vec3 ACESFilm_Advanced(vec3 color) {
    mat3 inputMatrix = mat3(
        0.59719, 0.07600, 0.02840,
        0.35458, 0.90834, 0.13383,
        0.04823, 0.01566, 0.83777
    );
    vec3 v = inputMatrix * color;
    
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    vec3 r = a / b;
    
    mat3 outputMatrix = mat3(
        1.60475, -0.10208, -0.00327,
        -0.53108,  1.10813, -0.07276,
        -0.07367, -0.00605,  1.07602
    );
    return clamp(outputMatrix * r, 0.0, 1.0);
}

void main() {
    vec3 hdrColor = texture(tScreenColorMap, iFragUV).rgb;
    
    // Bloom blur
    vec3 bloomColor = texture(tBloomblurMap, iFragUV).rgb;
    hdrColor = hdrColor + bloomColor * uBloomIntensity;
    
    // Lens flare
    vec3 flareColor = texture(tLensflareMap, iFragUV).rgb;
    vec3 dirtColor = texture(tDirtmaskMap, iFragUV).rgb;
    hdrColor = hdrColor + flareColor * dirtColor * uFlareIntensity;
    
    // Tone mapping(convert hdr color into sdr color)
    vec3 sdrColor = ACESFilm_Advanced(hdrColor);

    // Gamma correction
    vec3 finalColor = pow(sdrColor, vec3(1.0 / 2.2));

    oFragColor = vec4(finalColor, 1.0);
}