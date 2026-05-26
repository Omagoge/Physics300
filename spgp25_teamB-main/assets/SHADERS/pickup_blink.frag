#version 460 core

layout (location = 0) out vec4 gAlbedo;

in vec2 vTexCoord;
in vec3 vWorldPos;

uniform vec4 gColor;
uniform sampler2D gTexture;
uniform float time;

void main()
{
    vec4 texel = texture(gTexture, vTexCoord) * gColor;
    if (texel.a < 0.1)
        discard;

    // Blink
    float blink = 0.5 + 0.5 * sin(time * 6.2831853);

    vec3 blinkColor = mix(texel.rgb, vec3(1.0), blink * 0.75);

    gAlbedo = vec4(blinkColor, texel.a);
}

