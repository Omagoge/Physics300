#version 460

layout(location = 0) out vec4 oColor;

in vec2 TexCoord;

uniform sampler2D uInputTex;
uniform int uHorizontal;
uniform float uSigma;

float Gaussian(float x, float sigma) {
	return exp(-(x * x) / (2.0 * sigma * sigma));
}

void main() {
	vec2 texel = 1.0 / vec2(textureSize(uInputTex, 0));
	vec2 dir = (uHorizontal == 1) ? vec2(texel.x, 0.0) : vec2(0.0, texel.y);

	vec3 accum = vec3(0.0);
	float weightSum = 0.0;

	for (int i = -4; i <= 4; ++i) {
		float w = Gaussian(float(i), max(uSigma, 0.01));
		accum += texture(uInputTex, TexCoord + dir * float(i)).rgb * w;
		weightSum += w;
	}

	oColor = vec4(accum / max(weightSum, 0.0001), 1.0);
}

