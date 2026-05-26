#version 460 core

layout (location = 0) out vec4 gAlbedo;

in vec2 vTexCoord;
in vec2 vScreenUV;
in mat3 vTBN;

uniform vec4 gColor;
uniform sampler2D uSceneTex;
uniform sampler2D uNormalMap;

uniform float uRefractionStrengthX;
uniform float uRefractionStrengthY;
uniform float uBaseOffsetX;
uniform float uBaseOffsetY;
uniform float uNormalStrength;
uniform float uNormalTiling;
uniform float uOpacity;
uniform float uFlipY;
uniform float uWaveSpeed1;
uniform float uWaveSpeed2;
uniform float uDistortionMix;
uniform float uStraightDistortStrength;
uniform float uStraightBlend;
uniform float uVerticalPull;
uniform float uCausticStrength;
uniform float uCausticScale;
uniform float uCausticSpeed;
uniform float uCausticSharpness;
uniform float uTintMix;
uniform float time;

void main()
{
    vec2 baseUV = vTexCoord * max(uNormalTiling, 0.001);
    float t = time;
    vec2 uv1 = baseUV + vec2(uWaveSpeed1, uWaveSpeed1 * 0.63) * t;
    vec2 uv2 = baseUV * 1.73 + vec2(-uWaveSpeed2 * 0.7, uWaveSpeed2) * t;

    vec3 n1 = texture(uNormalMap, uv1).xyz * 2.0 - 1.0;
    vec3 n2 = texture(uNormalMap, uv2).xyz * 2.0 - 1.0;
    vec3 tangentNormal = normalize(mix(n1, n2, clamp(uDistortionMix, 0.0, 1.0)));
    tangentNormal.xy *= uNormalStrength;
    tangentNormal = normalize(tangentNormal);

    vec3 worldNormal = normalize(vTBN * tangentNormal);

    vec2 distort = worldNormal.xy;
    vec2 baseOffset = vec2(uBaseOffsetX, uBaseOffsetY);
    vec2 refractOffset = vec2(distort.x * uRefractionStrengthX, distort.y * uRefractionStrengthY);
    vec2 sceneUV = vScreenUV + baseOffset + refractOffset;
    vec2 straightUV = vScreenUV + baseOffset + distort * uStraightDistortStrength + vec2(0.0, abs(distort.y) * uVerticalPull);
    if (uFlipY > 0.5) {
        sceneUV.y = 1.0 - sceneUV.y;
        straightUV.y = 1.0 - straightUV.y;
    }
    sceneUV = clamp(sceneUV, vec2(0.001), vec2(0.999));
    straightUV = clamp(straightUV, vec2(0.001), vec2(0.999));

    vec4 refracted = texture(uSceneTex, sceneUV);
    vec4 straightSample = texture(uSceneTex, straightUV);

    float causticWave = sin((baseUV.x + distort.x * 0.35) * uCausticScale + t * uCausticSpeed)
                      * cos((baseUV.y + distort.y * 0.45) * uCausticScale * 0.9 - t * (uCausticSpeed * 1.17));
    float caustic = pow(clamp(0.5 + 0.5 * causticWave, 0.0, 1.0), max(uCausticSharpness, 0.01));

    float ndotv = clamp(abs(worldNormal.z), 0.0, 1.0);
    float fresnel = pow(1.0 - ndotv, 3.0);

    vec3 baseColor = mix(refracted.rgb, straightSample.rgb, clamp(uStraightBlend, 0.0, 1.0));
    vec3 tinted = mix(baseColor, baseColor * gColor.rgb, clamp(uTintMix, 0.0, 1.0));
    tinted *= 1.0 + caustic * uCausticStrength;
    tinted = mix(tinted, baseColor, fresnel * 0.25);

    gAlbedo = vec4(tinted, straightSample.a * gColor.a * uOpacity);
}





