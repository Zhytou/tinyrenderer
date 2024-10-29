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

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    float distance = length(uLightPos - vFragPos);
    float attenuation = 1.0; // (distance * distance);
    vec3 radiance = uLightRadiance * attenuation ;

    vec3 ambient = uKa * radiance;
    vec3 diffuse = uKd * radiance * NdotL;
    vec3 specular = uKs * radiance * pow(NdotH, uShininess);
    vec3 color = ambient + diffuse + specular;

    // HDR color -> LDR color
    color = color / (color + vec3(1.0));
    // gamma correction
    color = pow(color, vec3(1.0/2.2)); 
    gl_FragColor = vec4(uKd, 0.0);
}