#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aTangent;
uniform mat4 gMVP;
uniform mat4 gWorld;
out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vTangent;
out float vHandedness;
out vec3 vWorldPos;
out vec2 vTileUV;
void main()
{
    gl_Position = gMVP * vec4(aPos, 1.0);
    vTexCoord   = aTexCoord;
    vNormal     = (gWorld * vec4(aNormal,      0.0)).xyz;
    vTangent    = (gWorld * vec4(aTangent.xyz, 0.0)).xyz;
    vHandedness = aTangent.w;
    vWorldPos   = (gWorld * vec4(aPos, 1.0)).xyz;

    vec2 worldScale  = vec2(length(gWorld[0].xyz), length(gWorld[1].xyz));
    vec2 worldOrigin = gWorld[3].xy;

    vec2 worldUV = aPos.xy * worldScale + worldOrigin;

    float angle    = atan(gWorld[0].y, gWorld[0].x);
    float cosA     = cos(angle);
    float sinA     = sin(angle);
    vec2  pivotUV  = worldOrigin;   // rotate around the mesh's world position
    vec2  centered = worldUV - pivotUV;
    vTileUV = vec2(cosA * centered.x - sinA * centered.y,
                   sinA * centered.x + cosA * centered.y) + pivotUV;
}
