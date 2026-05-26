#version 460

layout(location = 0) out vec4 oColor;

in vec2 TexCoord;

uniform sampler2D uInputTex;

uniform float uExposure;
uniform float uGamma;

vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;

    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 color = texture(uInputTex, TexCoord).rgb;
    
    color *= uExposure;
    
    color = ACESFilm(color);
    
    // gamma
    color = pow(color, vec3(1.0 / uGamma));

    oColor = vec4(color, 1.0);
}