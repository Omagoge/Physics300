#version 460

layout(location = 0) out vec4 oColor;

in vec2 TexCoord;

uniform sampler2D uInputTex;
uniform sampler2D uBloomTex;
uniform float uIntensity;

void main() {
	vec3 base = texture(uInputTex, TexCoord).rgb;
	vec3 bloom = texture(uBloomTex, TexCoord).rgb;
	oColor = vec4(base + bloom * uIntensity, 1.0);
}

