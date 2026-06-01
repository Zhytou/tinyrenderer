


// Basic shadow mapping
float SM(vec2 uv, float z)
{
    float d = texture(uShadowMap, uv).r;
    float visibility = 0.0;
    if (z - EPSILON < d) {
        visibility = 1.0;
    }

    return visibility;
}

// Percentage closer filtering
float PCF(vec2 uv, float z, float range)
{
    generateRandom(uv);

    float visibility = 0.0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vec2 offset = poissonDisk[i] * range;
        visibility += SM(uv + offset, z);
    }

    return visibility / NUM_SAMPLES;
}

// Percentage closer soft shadow
float PCCS(vec2 uv, float z, float range)
{
    generateRandom(uv);

    // average depth of blockers
    float d = 0.0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vec2 offset = poissonDisk[i] * range;
        d += texture(uShadowMap, uv + offset).r;
    }
    d /= NUM_SAMPLES;

    // no shadow, just return 1.0
    if (z - EPSILON < d) {
        return 1.0;
    }

    // penumbra size
    float penumbraSize = LIGHT_SIZE * (z - d) / d;
    return PCF(uv, z, penumbraSize);
}


void main()
{
    vec3 N = calculateNormal();
    vec3 V = normalize(uCameraPos - vFragPos);
    vec3 R = reflect(-V, N);

    vec3 baseColor = texture(uAlbedoMap, vFragUV).rgb;
    float roughness = uRoughnessMapped ? texture(uRoughnessMap, vFragUV).r : 1.0;
    float metallic  = uMetallicMapped  ? texture(uMetallicMap,  vFragUV).r : 0.0;
    float ao        = uAOMapped        ? texture(uAOMap,         vFragUV).r : 1.0;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, baseColor, metallic);

    vec3 Lo = vec3(0.0);

    // point light shading
    for (int i = 0; i < uPointLightNum; i++) {
        float distance = length(uPointLights[i].position - vFragPos);
        float attenuation = 1.0 / (distance * distance);

        vec3 L = normalize(uPointLights[i].position - vFragPos);
        vec3 brdf = BRDF(L, V, N, F0, baseColor, metallic, roughness, true);

        float NdotL = clamp(dot(N, L), 0.0, 1.0);

        Lo += uPointLights[i].color * attenuation * brdf * NdotL;
    }

    // directional light shading
    {
        vec3 projCoords = vLightSpaceFragPos.xyz / vLightSpaceFragPos.w;
        projCoords = projCoords * 0.5 + 0.5;

        float visibility = PCCS(projCoords.xy, projCoords.z, 0.02);
        if (visibility > 0.0) {
            vec3 L = normalize(-uDirectionalLight.direction);
            vec3 brdf = BRDF(L, V, N, F0, baseColor, metallic, roughness, false);

            float NdotL = clamp(dot(N, L), 0.0, 1.0);

            Lo += uDirectionalLight.color * brdf * NdotL * visibility;
        }
    }


    gl_FragColor = vec4(color, 1.0);
}