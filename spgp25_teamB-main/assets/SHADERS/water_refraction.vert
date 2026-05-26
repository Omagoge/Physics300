#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aTangent; // xyz = tangent, w = handedness

uniform mat4 gMVP;
uniform mat4 gWorld;

out vec2 vTexCoord;
out vec2 vScreenUV;
out mat3 vTBN;

void main()
{
    vec4 clipPos = gMVP * vec4(aPos, 1.0);
    gl_Position = clipPos;

    vTexCoord = aTexCoord;
    vScreenUV = (clipPos.xy / clipPos.w) * 0.5 + 0.5;

    vec3 n = normalize((gWorld * vec4(aNormal, 0.0)).xyz);
    vec3 t = normalize((gWorld * vec4(aTangent.xyz, 0.0)).xyz);
    vec3 b = normalize(cross(n, t) * aTangent.w);

    vTBN = mat3(t, b, n);
}

