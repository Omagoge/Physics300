#version 460

layout(location = 0) out vec4 oColor;

in vec2 TexCoord;

uniform sampler2D uInputTex;
uniform float uThreshold;
uniform float uKnee;

void main() {
	vec3 hdr = texture(uInputTex, TexCoord).rgb;
	float brightness = max(max(hdr.r, hdr.g), hdr.b);
	float knee = max(0.0001, uThreshold * uKnee);
	float soft = clamp((brightness - uThreshold + knee) / (2.0 * knee), 0.0, 1.0);
	float weight = max(brightness - uThreshold, 0.0) + soft * soft * knee;
	weight /= max(brightness, 0.0001);
	oColor = vec4(hdr * weight, 1.0);
}

