#ifndef COMMON_BRDF_GLSL
#define COMMON_BRDF_GLSL

#define PI 3.1415926535897932384626
#define EPSILON 1e-6

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

vec3 BRDF(vec3 L, vec3 V, vec3 N, vec3 F0, vec3 baseColor, float metallic, float roughness)
{
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

    vec3 diffuse = baseColor * NdotL;
    return diffuse;
    // if (isPointLight) {
    //     diffuse /= PI;
    // }
    // vec3 specular = D * F * G / (4.0 * NdotL * NdotV + EPSILON);

    // return (diffuse + specular) * NdotL;
}

#endif
