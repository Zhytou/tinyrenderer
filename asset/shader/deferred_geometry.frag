#version 450

#include "common_normal.glsl"

layout(location = 0) in vec3 iFragNormal;
layout(location = 1) in vec3 iFragTangent;
layout(location = 2) in vec2 iFragUV;

layout(binding = 0) uniform sampler2D tAlbedoMap;
layout(binding = 1) uniform sampler2D tNormalMap;
layout(binding = 2) uniform sampler2D tMRAOMap;

layout(location = 0) out vec4 oFragAlbedo;
layout(location = 1) out vec4 oFragNormal;
layout(location = 2) out vec4 oFragMRAO;

void main() {
    oFragAlbedo = vec4(texture(tAlbedoMap, iFragUV).rgb, 0.0);
    oFragNormal = vec4(N_encode(N_toWorld(
        iFragNormal, 
        iFragTangent, 
        N_decode(texture(tNormalMap, iFragUV).xyz)
    )), 0.0);
    oFragMRAO   = texture(tMRAOMap, iFragUV);
}