#version 450

#include "common_sampling.glsl"

layout(location = 0) in vec3 iFragDir;

uniform float uRoughness;
layout(binding = 16) uniform samplerCube tSkyboxMap;

out vec4 oFragColor;

void main() {
    const int numSamples = 1024;

    vec3 N = normalize(iFragDir);
    vec3 V = N; // assume N is the view direction for less calculation
    vec3 R = V;
    
    vec3 prefilteredColor = vec3(0.0);
    float weight = 0.0;
    for (int i = 0; i < numSamples; i++) {
        vec2 X = HammersleySample(i, numSamples);
        vec3 H = GGXSample(X, N, uRoughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0) {
            prefilteredColor += texture(tSkyboxMap, L).rgb * NdotL;
            weight += NdotL;
        }
    }

    oFragColor = vec4(prefilteredColor / weight, 1.0);
}