#version 450

layout(location = 0) in vec3 iFragDir;

layout(binding = 21) uniform sampler2D tSkyboxEquirectMap;

out vec4 oFragColor;

void main() {
    vec3 dir = normalize(iFragDir);
    float theta = atan(dir.z, dir.x);    // [-π, π]
    float phi   = asin(dir.y);           // [-π/2, π/2]

    float pi = 3.14159265359;
    vec2 uv;
    uv.x = theta / (2.0 * pi) + 0.5;
    uv.y = phi / pi + 0.5;

    oFragColor = vec4(texture(tSkyboxEquirectMap, uv).rgb, 1.0);
}