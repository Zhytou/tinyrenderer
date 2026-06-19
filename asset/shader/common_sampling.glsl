#ifndef COMMON_SAMPLING_GLSL
#define COMMON_SAMPLING_GLSL

// Generates a sample point distributed over multiple concentric rings on a 2D disk for shadow map filtering (e.g., PCF/PCSS).
vec2 PoissonSample(int i, vec2 seed, int numSamples, int numRings) {
    const float PI = 3.1415926535897932384626;
    
    float angleStep = 2.0 * PI * float(numRings) / float(numSamples);
    float radiusStep = 1.0 / float(numSamples);

    float angle = sin(seed.x + seed.y) * 2.0 * PI + angleStep * float(i);
    float radius = radiusStep * float(i);

    vec2 sampleVec = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
    return sampleVec;
}

// Generates a low-discrepancy 2D sequence in the [0, 1]^2 range via radical inverse bit-shiftting for Quasi-Monte Carlo integration.
vec2 HammersleySample(int i, int numSamples) {
    uint bits = uint(i);
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float radicalInverse = float(bits) * 2.3283064365386963e-10; // / 0x100000000
    return vec2(float(i) / float(numSamples), radicalInverse);
}

// Transforms a 2D uniform random sample into a 3D world-space microfacet normal vector matching the GGX probability density function.
vec3 GGXSample(vec2 X, vec3 N, float roughness) {
    const float PI = 3.1415926535897932384626;
    float a = roughness*roughness;

    // X is random vec2 in [0, 1] range
    float phi = 2.0 * PI * X.x;
    float cosTheta = sqrt((1.0 - X.y) / (1.0 + (a*a - 1.0) * X.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

#endif