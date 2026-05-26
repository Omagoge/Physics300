#version 460

layout(location = 0) out vec4 oColor;

in vec2 TexCoord;

uniform sampler2D uInputTex;
uniform float uThreshold;
uniform float uIntensity;

void main() {
	vec3 color = texture(uInputTex, TexCoord).rgb;
	float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float bloomMask = smoothstep(uThreshold, uThreshold + 1.0, luminance);
	vec3 bloom = color * bloomMask * uIntensity;
	oColor = vec4(color + bloom, 1.0);
}

