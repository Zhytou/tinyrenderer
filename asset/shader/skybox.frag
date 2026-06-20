#version 450

layout(location = 0) in vec3 iFragDir;

layout(binding = 16) uniform samplerCube tSkyboxMap;

out vec4 oFragColor;

void main() {
    oFragColor = texture(tSkyboxMap, normalize(iFragDir));
}
