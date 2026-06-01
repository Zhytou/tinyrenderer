#version 450

layout(location = 0) in vec3 vFragPos;
layout(location = 1) in vec3 vFragNormal;
layout(location = 2) in vec3 vFragTangent;
layout(location = 3) in vec2 vFragUV;
layout(location = 4) in vec4 vLightSpaceFragPos;

layout(std140, binding = 0) uniform LightBlock {
    mat4 uLightSpaceMatrix; 
    vec3 uLightDirection;
    vec3 uLightColor;
};
layout(std140, binding = 1) uniform CameraBlock {
    mat4 uViewMatrix;
    mat4 uProjectMatrix;
    vec3 uCameraPos;
};

layout(location = 0) out vec3 gFragPosition;
layout(location = 1) out vec3 gFragNormal;
layout(location = 2) out vec3 gFragAlbedo;
layout(location = 3) out vec3 gFragMRAO;

vec3 calculateNormal() {
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

void main() {
    gFragPosition = vFragPos;
    gFragNormal = calculateNormal();
    // gFragAlbedo = uAlbedo;
    // gFragMRAO = uMRAO;
}