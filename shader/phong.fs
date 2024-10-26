#version 330

in vec3 vFragPos;
in vec3 vFragNormal;

uniform vec3 uLightPos;
uniform vec3 uLightRadiance;
uniform vec3 uCameraPos;

uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;

void main() {
    vec3 N = normalize(vFragNormal);
    vec3 V = normalize(uCameraPos - vFragPos);
    vec3 L = normalize(uLightPos - vFragPos);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    vec3 ambient = uKa * uLightRadiance;
    vec3 diffuse = uKd * uLightRadiance * NdotL;
    vec3 specular = uKs * uLightRadiance * pow(NdotH, uShininess);
    vec3 color = ambient + diffuse + specular;
    gl_FragColor = vec4(color, 0.0);
}