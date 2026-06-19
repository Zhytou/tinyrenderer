#ifndef COMMON_SHADOW_GLSL
#define COMMON_SHADOW_GLSL

#include "common_sampling.glsl"

// World space position of the fragment in light's view space
vec3 Pos_toLightSpaceUVD(mat4 lightViewProjMatrix, vec3 worldPos) {
    vec4 lightSpacePos = lightViewProjMatrix * vec4(worldPos, 1.0);
    vec3 lightSpaceUVD = (lightSpacePos.xyz / lightSpacePos.w + 1.0) / 2.0;
    lightSpaceUVD.z = clamp(lightSpaceUVD.z, 0.0, 1.0);
    return lightSpaceUVD;
}

// Basic shadow mapping
float SM(sampler2D shadowMap, vec2 uv, float depth) {
    const float bias = 0.05;
    float refDepth = texture(shadowMap, uv).x; // reference depth value
    return (depth - bias < refDepth) ? 1.0 : 0.0; // visibility
}

// Percentage closer filtering
float PCF(sampler2D shadowMap,vec2 uv, float z, float range) {
    const int numSamples = 32;
    const int numRings = 4;

    float visibility = 0.0;
    for (int i = 0; i < numSamples; i++) {
        vec2 x = PoissonSample(i, uv, numSamples, numRings);
        vec2 offset = x * range;
        visibility += SM(shadowMap, uv + offset, z);
    }

    return visibility / numSamples;
}

// Percentage closer soft shadow
float PCCS(sampler2D shadowMap,vec2 uv, float z, float range) {
    const int numSamples = 32;
    const int numRings = 4;
    
    // average depth of blockers
    float d = 0.0;
    for (int i = 0; i < numSamples; i++) {
        vec2 x = PoissonSample(i, uv, numSamples, numRings);
        vec2 offset = x * range;
        d += texture(shadowMap, uv + offset).r;
    }
    d /= numSamples;

    // no shadow, just return 1.0
    const float bias = 0.005;
    if (z - bias < d) {
        return 1.0;
    }

    const float lightSize = 0.02; // light size, control soft shadow range
    float penumbraSize = lightSize * (z - d) / d; // penumbra size
    return PCF(shadowMap, uv, z, penumbraSize);
}

#endif