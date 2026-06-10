#ifndef COMMON_NORMAL_GLSL
#define COMMON_NORMAL_GLSL

// Get normal matrix from transform matrix
mat4 N_matrix(mat4 transformMatrix) {
    return mat4(transpose(inverse(mat3(transformMatrix))));
}

// Transform tangent-space normal from normal map to world space
vec3 N_toWorld(vec3 N, vec3 T, vec3 TN) {
    vec3 B = normalize(cross(N, T));

    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * TN);
}

// Decode normal from GBuffer/Texture storage format[0.0, 1.0] to shader format[-1, 1]
vec3 N_decode(vec3 N) {
    return N * 2.0 - 1.0;
}

// Encode normal from shader format[-1, 1] to GBuffer/Texture storage format[0.0, 1.0]
vec3 N_encode(vec3 N) {
    return N * 0.5 + 0.5;
}

#endif
