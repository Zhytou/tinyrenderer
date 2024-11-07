#version 330

#define PI 3.14159265359

in vec3 vFragPos;
in vec3 vFragNormal;
in vec3 vFragTangent;
in vec2 vFragUV;
in vec4 vLightSpaceFragPos;

uniform sampler2D uShadowMap;

uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uMetallicMap;
uniform sampler2D uRoughnessMap;
uniform sampler2D uAOMap;

uniform bool uNormalMapped;
uniform bool uMetallicMapped;
uniform bool uRoughnessMapped;
uniform bool uAOMapped;

uniform vec3 uCameraPos;

struct PointLight {
    vec3 position;
    vec3 color;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

#define MAX_POINT_LIGHT_NUM 10

uniform int uPointLightNum;
uniform PointLight uPointLights[MAX_POINT_LIGHT_NUM];
uniform DirectionalLight uDirectionalLight;

#define EPSILON 0.001

// macros for shadow mapping
#define NUM_SAMPLES 20
#define NUM_RINGS 10
#define LIGHT_SIZE 0.01

vec3 calculateNormal()
{
    if (!uNormalMapped) {
        return normalize(vFragNormal);
    }
    
    vec3 tangentNormal = texture(uNormalMap, vFragUV).xyz * 2.0 - 1.0;

    vec3 N = normalize(vFragNormal);
    vec3 T = normalize(vFragTangent);
    vec3 B = normalize(cross(N, T));

    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}

vec2 poissonDisk[NUM_SAMPLES];

// Generate a random vec2 sequence with poisson disk sampling
void generateRandom(const in vec2 seed)
{

  float ANGLE_STEP = 2.0 * PI * float(NUM_RINGS) / float(NUM_SAMPLES);
  float INV_NUM_SAMPLES = 1.0 / float(NUM_SAMPLES);

  float angle = sin(seed.x + seed.y) * 2.0 * PI;
  float radius = INV_NUM_SAMPLES;
  float radiusStep = radius;

  for(int i = 0; i < NUM_SAMPLES; i++) {
    poissonDisk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
    radius += radiusStep;
    angle += ANGLE_STEP;
  }
}

// Basic shadow mapping
float SM(vec2 uv, float z)
{
    float d = texture(uShadowMap, uv).r;
    float visibility = 0.0;
    if (z - EPSILON < d) {
        visibility = 1.0;
    }

    return visibility;
}

// Percentage closer filtering
float PCF(vec2 uv, float z, float range)
{
    generateRandom(uv);

    float visibility = 0.0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vec2 offset = poissonDisk[i] * range;
        visibility += SM(uv + offset, z);
    }

    return visibility / NUM_SAMPLES;
}

// Percentage closer soft shadow
float PCCS(vec2 uv, float z, float range)
{
    generateRandom(uv);

    // average depth of blockers
    float d = 0.0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vec2 offset = poissonDisk[i] * range;
        d += texture(uShadowMap, uv + offset).r;
    }
    d /= NUM_SAMPLES;

    // no shadow, just return 1.0
    if (z - EPSILON < d) {
        return 1.0;
    }

    // penumbra size
    float penumbraSize = LIGHT_SIZE * (z - d) / d;
    return PCF(uv, z, penumbraSize);
}

float D_GGX(float NdotH, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return num / denom;
}

vec3 F_Schlick(float NdotV, vec3 F0)
{
    return F0 + (vec3(1.0) - F0) * pow(1.0 - NdotV, 5.0);
}

float G_SchlicksmithGGX(float NdotL, float NdotV, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float GL = NdotL / (NdotL * (1.0 - k) + k);
    float GV = NdotV / (NdotV * (1.0 - k) + k);

    return GL * GV;
}

vec3 BRDF(vec3 L, vec3 V, vec3 N, vec3 F0, vec3 baseColor, float metallic, float roughness, bool isPointLight)
{
    if (!uMetallicMapped || !uRoughnessMapped) {
        return baseColor;
    }

    // Precalculate vectors and dot products
    vec3  H     = normalize(V + L);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float NdotL = clamp(dot(N, L), 0.0, 1.0);
    float NdotV = clamp(dot(N, V), 0.0, 1.0);
    float HdotV = clamp(dot(H, V), 0.0, 1.0);

    // D = Normal distribution (Distribution of the microfacets)
    float D = D_GGX(NdotH, roughness);
    // G = Geometric shadowing term (Microfacets shadowing)
    float G = G_SchlicksmithGGX(NdotL, NdotV, roughness);
    // F = Fresnel factor (Reflectance depending on angle of incidence)
    vec3 F = F_Schlick(HdotV, F0);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 diffuse = kD * baseColor;
    if (isPointLight) {
        diffuse /= PI;
    }
    vec3 specular = D * F * G / (4.0 * NdotL * NdotV + EPSILON);

    return diffuse + specular;
}

void main()
{
    vec3 N = calculateNormal();
    vec3 V = normalize(uCameraPos - vFragPos);
    
    vec3 baseColor = texture(uAlbedoMap, vFragUV).rgb;
    // baseColor = pow(baseColor, vec3(2.2));
    float roughness = texture(uRoughnessMap, vFragUV).r;
    float metallic = texture(uMetallicMap, vFragUV).r;
    float ao = texture(uAOMap, vFragUV).r;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, baseColor, metallic);

    vec3 color = vec3(0.0);

    // point light shading
    for (int i = 0; i < uPointLightNum; i++) {
        float distance = length(uPointLights[i].position - vFragPos);
        float attenuation = 1 / (distance * distance);

        vec3 L = normalize(uPointLights[i].position - vFragPos);
        vec3 brdf = BRDF(L, V, N, F0, baseColor, metallic, roughness, true);

        float NdotL = clamp(dot(N, L), 0.0, 1.0);

        color += uPointLights[i].color * attenuation * brdf * NdotL;
    }

    // directional light shading
    {
        // camera(here light is camera) space -> clip space
        vec3 projCoords = vLightSpaceFragPos.xyz / vLightSpaceFragPos.w;
        // pointlight(perspective projection) need depth linearization
        // since we only calculate the visibility of directional light here
        // we don't need to linearize the depth
        // [-1, 1] -> [0, 1]
        projCoords = projCoords * 0.5 + 0.5;

        float visibility = PCCS(projCoords.xy, projCoords.z, 0.02);
        if (visibility > 0) {
            vec3 L = normalize(-uDirectionalLight.direction);
            vec3 brdf = BRDF(L, V, N, F0, baseColor, metallic, roughness, false);

            float NdotL = clamp(dot(N, L), 0.0, 1.0);

            color += uDirectionalLight.color * brdf * NdotL * visibility;
        }
    }

    // if (uAOMapped) {
    //     color += vec3(0.03) * baseColor * ao;
    // }

    // gamma correction
    // color = pow(color, vec3(1.0 / 2.2));

    gl_FragColor = vec4(color, 1.0);
}