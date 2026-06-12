#ifndef COMMON_SHADOW_GLSL
#define COMMON_SHADOW_GLSL

#ifndef PI
#define PI 3.1415926535897932384626
#endif

#ifndef BIAS_EPSILON
#define BIAS_EPSILON     0.0005    // shadow bias
#endif
#define LIGHT_SIZE  0.02      // light size, control soft shadow range

#define NUM_SAMPLES 32 // number of samples for PCF and PCCS, more samples means smoother shadow but worse performance
#define NUM_RINGS 4 // number of rings for poisson disk sampling, control the distribution of samples, more rings means more uniform distribution but worse performance

vec2 disk[NUM_SAMPLES];

// Generate a random vec2 sequence with poisson disk sampling
void Poisson_sampling(const in vec2 seed) {

    float angleStep = 2.0 * PI * float(NUM_RINGS) / float(NUM_SAMPLES);
    float invSample = 1.0 / float(NUM_SAMPLES);

    float angle = sin(seed.x + seed.y) * 2.0 * PI;
    float radius = invSample;
    float radiusStep = radius;

    for (int i = 0; i < NUM_SAMPLES; ++i) {
        disk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
        radius += radiusStep;
        angle += angleStep;
    }
}

// World space position of the fragment in light's view space
vec3 Pos_toLightSpaceUVD(mat4 lightViewProjMatrix, vec3 worldPos) {
    vec4 lightSpacePos = lightViewProjMatrix * vec4(worldPos, 1.0);
    vec3 lightSpaceUVD = (lightSpacePos.xyz / lightSpacePos.w + 1.0) / 2.0;
    lightSpaceUVD.z = clamp(lightSpaceUVD.z, 0.0, 1.0);
    return lightSpaceUVD;
}

// Basic shadow mapping
float SM(sampler2D shadowMap, vec2 uv, float depth) {
    float refDepth = texture(shadowMap, uv).x; // reference depth value
    return (depth - BIAS_EPSILON < refDepth) ? 1.0 : 0.0; // visibility
}

// Percentage closer filtering
float PCF(sampler2D shadowMap,vec2 uv, float z, float range) {
    Poisson_sampling(uv);

    float visibility = 0.0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vec2 offset = disk[i] * range;
        visibility += SM(shadowMap, uv + offset, z);
    }

    return visibility / NUM_SAMPLES;
}

// Percentage closer soft shadow
float PCCS(sampler2D shadowMap,vec2 uv, float z, float range) {
    Poisson_sampling(uv);

    // average depth of blockers
    float d = 0.0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vec2 offset = disk[i] * range;
        d += texture(shadowMap, uv + offset).r;
    }
    d /= NUM_SAMPLES;

    // no shadow, just return 1.0
    if (z - BIAS_EPSILON < d) {
        return 1.0;
    }

    // penumbra size
    float penumbraSize = LIGHT_SIZE * (z - d) / d;
    return PCF(shadowMap, uv, z, penumbraSize);
}

#endif