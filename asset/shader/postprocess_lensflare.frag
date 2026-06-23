#version 450

layout(location = 0) in vec2 iFragUV;

uniform float uDispersion = 0.3;
uniform float uDistortion = 0.02;
uniform int   uGhostSamples = 3;
uniform float uHaloRadius = 0.45;
uniform float uHaloThickness = 0.6;

layout(binding = 25) uniform sampler2D tHighlightMap;
layout(binding = 27) uniform sampler2D tBloomblurMap;

out vec4 oFragColor;

vec3 textureDistorted(sampler2D map, vec2 uv, vec2 distortion) {
    vec3 color = vec3(0.0);
    color.r = texture(map, uv + distortion).r;
    color.g = texture(map, uv).g;
    color.b = texture(map, uv - distortion).b;
    return color;
}

void main() {
    // ----------------------------------------------------------------
    // Initialize
    // ----------------------------------------------------------------
    vec2 uv = iFragUV;
    vec2 dir = vec2(0.5) - uv; // uv -> center directional vector
    vec2 ndir = normalize(dir); // normalized uv -> center directional vector
    
    const vec2 distortion = uDistortion * dir;
    const float dispersion = uDispersion;
    const int samples = uGhostSamples;
    const float radius = uHaloRadius;
    const float thickness = uHaloThickness;

    // ----------------------------------------------------------------
    // Generate ghost flare
    // ----------------------------------------------------------------
    vec3 ghostColor = vec3(0.0);
    for (int i = 0; i < samples; ++i) {
        // calculate sampling texcoord position
        vec2 ghostUV = fract(vec2(0.5) + dir * (i + 1) * dispersion);

        // calculate sampling weight
        float ghostWeight = length(ghostUV - vec2(0.5)) / length(vec2(0.5));
        ghostWeight = pow(1 - ghostWeight, 10);
        
        // distort sampling color and calculate weighted sum
        ghostColor += textureDistorted(tBloomblurMap, ghostUV, distortion) * ghostWeight;
    }

    // ----------------------------------------------------------------
    // Generate halo
    // ----------------------------------------------------------------
    vec3 haloColor = vec3(0.0);
    {
        vec2 haloUV = fract(uv + ndir * radius);

        float haloWeight = length(haloUV - vec2(0.5)) / length(vec2(0.5));
        haloWeight = pow(1 - haloWeight, 10);
    
        haloColor = textureDistorted(tBloomblurMap, haloUV, distortion) * haloWeight;
    }
    
    oFragColor = vec4(ghostColor + haloColor, 1.0);
}