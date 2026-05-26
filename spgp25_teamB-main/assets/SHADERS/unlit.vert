#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aTangent; // xyz = tangent, w = handedness

uniform mat4 gMVP;
uniform mat4 gWorld;

uniform vec3 lightColor = vec3(1,1,1);

out vec2 vTexCoord;

void main() {
	gl_Position = gMVP * vec4(aPos, 1.0);
	vTexCoord = aTexCoord;
}
