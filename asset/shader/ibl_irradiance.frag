#version 450

layout(location = 0) in vec3 iFragDir;

layout(binding = 16) uniform samplerCube tSkyboxMap;

out vec4 oFragColor;

void main() {
    const float PI = 3.14159265359;
    const float Delta = 0.25; 

    vec3 front = normalize(iFragDir);
    vec3 up    = vec3(0.0, 1.0, 0.0);
    if(abs(front.y) > 0.999) up = vec3(1.0, 0.0, 0.0);
    vec3 right = normalize(cross(front, up));
    up         = normalize(cross(right, front));

    int numSamples = 0; 
    vec3 irradiance = vec3(0.0);
    for(float phi = 0.0; phi < 2.0 * PI; phi += Delta) {
        for(float theta = 0.0; theta < 0.5 * PI; theta += Delta) {
            vec3 sampleVec = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            
            // Convert into world space
            sampleVec = sampleVec.x * right + sampleVec.y * up + sampleVec.z * front; 
            
            // Sample environment map
            irradiance += texture(tSkyboxMap, sampleVec).rgb * cos(theta) * sin(theta);
            
            numSamples++;
        }
    }
    oFragColor = vec4(PI * irradiance / float(numSamples), 1.0);
}