#version 460

layout(location = 0) out vec4 oLightingAcum;

uniform ivec2 screenSize;

uniform vec2 lightPosUV;
uniform float invLightRadius;

uniform vec2 correctedLightDir;

uniform vec4 gLightColor;
uniform float gLightIntensity;
uniform float gLightVolumetricIntensity;

uniform float cosOuter;
uniform float cosInner;

uniform float gRadialSoftness;
uniform float gAngularSoftness;

uniform int gShadowSteps;
uniform float invShadowRadius;

uniform bool doShadows;

uniform sampler2D sdfTex;

float interleavedGradientNoise(vec2 p) {
    return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

void main()
{
    vec2 screenUV = gl_FragCoord.xy / vec2(screenSize);

    vec2 delta = screenUV - lightPosUV;

    float aspect = float(screenSize.x) / float(screenSize.y);
    delta.x *= aspect;

    float distToLight = length(delta);

    if (distToLight > (1.0 / invLightRadius)) {
        discard;
    }

    float dNorm = distToLight * invLightRadius;

    vec2 fragDir = delta / distToLight;

    float radial = clamp(1.0 - dNorm, 0.0, 1.0);
    float radialFalloff = radial * radial; // pow(x,2) -> x*x
    radialFalloff *= smoothstep(1.0, 1.0 - gRadialSoftness, dNorm);

    float cosTheta = dot(fragDir, correctedLightDir);
    float angularFalloff = smoothstep(cosOuter, cosInner, cosTheta);

    float attenuation = radialFalloff * angularFalloff;

    // raymarch
    float shadow = 1.0;

    if (doShadows) {
        vec2 rayDirUV = normalize(lightPosUV - screenUV);
        float maxDistUV = length(lightPosUV - screenUV);

        float t = 0.001 + interleavedGradientNoise(gl_FragCoord.xy) * 0.002;

        for (int i = 0; i < gShadowSteps; ++i) {
            vec2 rayPos = screenUV + rayDirUV * t;
            float sdfDist = texture(sdfTex, rayPos).r;

            if (sdfDist < 0.0005) {
                shadow = 0.0;
                break;
            }

            float currentShadow = sdfDist * (invShadowRadius / t);
            shadow = min(shadow, currentShadow);

            t += max(sdfDist * 0.8, 0.001);

            if (t >= maxDistUV || shadow <= 0.0)
            break;
        }

        shadow = clamp(shadow, 0.0, 1.0);
        shadow = smoothstep(0.0, 1.0, shadow);
    }

    vec3 lightBase = gLightColor.rgb * gLightIntensity;

    vec3 finalColor = lightBase * attenuation * shadow;

    float volumetricShadow = mix(0.2, 1.0, shadow);
    finalColor += lightBase * gLightVolumetricIntensity * attenuation * volumetricShadow;

    oLightingAcum = vec4(finalColor, attenuation * shadow);
}