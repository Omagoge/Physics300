#version 460

layout(location = 0) out vec4 oColor;

in vec2 TexCoord;

uniform sampler2D uInputTex;

uniform float uContrast;
uniform float uSaturation;
uniform vec3 uTint;

uniform vec3 uLift;
uniform vec3 uGamma;
uniform vec3 uGain;

vec3 ApplyLGG(vec3 color, vec3 lift, vec3 gamma, vec3 gain)
{

    color *= gain;

    color += lift * (1.0 - color);

    color = pow(max(color, vec3(0.0)), 1.0 / max(gamma, vec3(0.01)));

    return color;
}

void main()
{
    vec4 tex = texture(uInputTex, TexCoord);
    vec3 color = tex.rgb;

    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luma), color, uSaturation);
    color = (color - 0.18) * uContrast + 0.18;

    color = ApplyLGG(color, uLift, uGamma, uGain);

    color *= uTint;

    oColor = vec4(clamp(color, 0.0, 1.0), tex.a);
}