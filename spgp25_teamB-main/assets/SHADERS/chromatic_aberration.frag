#version 460

layout(location = 0) out vec4 oColor;

in vec2 TexCoord;

uniform sampler2D uInputTex;
uniform float uStrength;
uniform float uFalloff;

void main() {
	vec2 centered = TexCoord * 2.0 - 1.0;
	float radius = length(centered);
	vec2 dir = radius > 0.0001 ? centered / radius : vec2(0.0);
	float amount = uStrength * pow(clamp(radius, 0.0, 1.0), uFalloff);
	vec2 offset = dir * amount;

	float r = texture(uInputTex, TexCoord + offset).r;
	float g = texture(uInputTex, TexCoord).g;
	float b = texture(uInputTex, TexCoord - offset).b;
	oColor = vec4(r, g, b, 1.0);
}

