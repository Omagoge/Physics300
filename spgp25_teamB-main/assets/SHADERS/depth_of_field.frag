#version 460

layout(location = 0) out vec4 oColor;

in vec2 TexCoord;

uniform sampler2D uInputTex;
uniform sampler2D uDepthTex;
uniform float uFocusDistance;
uniform float uFocusRange;
uniform float uBlurStrength;
uniform float uNearBlurScale;
uniform float uFarBlurScale;
uniform float uNearPlane;
uniform float uFarPlane;
uniform int uUseDepth;

float LinearizeDepth(float depthSample) {
	float z = depthSample * 2.0 - 1.0;
	return (2.0 * uNearPlane * uFarPlane) / (uFarPlane + uNearPlane - z * (uFarPlane - uNearPlane));
}

void main() {
	vec3 center = texture(uInputTex, TexCoord).rgb;
	float depth = (uUseDepth == 1) ? LinearizeDepth(texture(uDepthTex, TexCoord).r) : uFocusDistance;
	float signedCoc = (depth - uFocusDistance) / max(uFocusRange, 0.0001);
	float nearCoc = max(-signedCoc, 0.0) * uNearBlurScale;
	float farCoc = max(signedCoc, 0.0) * uFarBlurScale;
	float coc = nearCoc + farCoc;
	float radius = min(coc * uBlurStrength, 12.0);

	vec2 texel = 1.0 / vec2(textureSize(uInputTex, 0));
	vec3 accum = center;
	float weight = 1.0;

	for (int y = -2; y <= 2; ++y) {
		for (int x = -2; x <= 2; ++x) {
			if (x == 0 && y == 0) {
				continue;
			}
			vec2 offset = vec2(float(x), float(y));
			float dist = length(offset);
			if (dist > 2.0) {
				continue;
			}
			float tapWeight = exp(-dist * dist * 0.7);
			vec2 uv = TexCoord + offset * texel * radius;
			accum += texture(uInputTex, uv).rgb * tapWeight;
			weight += tapWeight;
		}
	}

	oColor = vec4(accum / weight, 1.0);
}



