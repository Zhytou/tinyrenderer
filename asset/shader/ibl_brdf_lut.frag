#version 450

#include "common_brdf.glsl"
#include "common_sampling.glsl"

layout(location = 0) in vec2 iFragUV;

out vec4 oFragColor;

// Integrate BRDF 
vec2 IntegrateBRDF(float NdotV, float roughness) {
    const float PI = 3.14159265358979323846;
    const int numSamples = 1024;
    vec3 N = vec3(0.0, 0.0, 1.0);
    vec3 V = vec3(sqrt(1.0 - NdotV*NdotV), 0.0, NdotV);

    float A = 0.0;
    float B = 0.0;
    for(int i = 0; i < numSamples; ++i) {
        vec2 X = HammersleySample(i, numSamples);
        vec3 H = GGXSample(X, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0) {
            float k = (roughness * roughness) / 2.0;
            float G = G_SchlicksmithGGX(NdotL, NdotV, k);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(numSamples);
    B /= float(numSamples);
    
    return vec2(A, B);
}

void main() {
    // brdf lut(x is NdotV, y is roughness)
    oFragColor = vec4(IntegrateBRDF(iFragUV.x, iFragUV.y), 0.0, 1.0);
}