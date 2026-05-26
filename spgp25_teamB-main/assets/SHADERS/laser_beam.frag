#version 460 core

in vec4 vColor;
in vec2 vBeamUV;
out vec4 FragColor;

uniform float u_GlowIntensity;

void main() {
    float lat = abs(vBeamUV.x);

    float coreWidth = 0.12;

    float uvF = max(fwidth(vBeamUV.x), 1e-6);
    float screenPerUV = fwidth(gl_FragCoord.x) / uvF;
    float desiredCorePixels = 4.0;
    float pixelDist = abs(vBeamUV.x) * screenPerUV;
    float core = 1.0 - smoothstep(desiredCorePixels * 0.5 - 0.5, desiredCorePixels * 0.5 + 0.5, pixelDist);

    float glowStart = 0.2;
    float glowEnd = 1.0;
    float glow = 1.0 - smoothstep(glowStart, glowEnd, lat);

    vec3 coreTint = mix(vColor.rgb, vec3(1.0, 1.0, 1.0), 0.35);
    vec3 coreCol = coreTint * core * 1.0;
    vec3 glowCol = vColor.rgb * glow * u_GlowIntensity;

    float edgeFalloff = 1.0 - smoothstep(0.0, glowEnd, lat);
    edgeFalloff = pow(edgeFalloff, 1.2);

    float outAlpha = vColor.a * clamp(core + glow * 0.6, 0.0, 1.0) * edgeFalloff;

    vec3 result = coreCol + glowCol;
    result = clamp(result, 0.0, 1.0);

    FragColor = vec4(result, outAlpha);
    if (FragColor.a < 0.003) discard;
}