#version 460 core

// Forward-shaded output
layout (location = 0) out vec4 gAlbedo;

in vec2 vTexCoord;
in vec3 vWorldPos;
in vec2 vTileUV;

uniform vec4  gColor;
uniform vec4  gLineColor;
uniform float gGridSize;
uniform float gLineWidth;

void main()
{
    // Compute which cell we are in
    vec2 uv = vTileUV / gGridSize;

    // Distance to the nearest grid line in each axis
    vec2 grid = abs(fract(uv - 0.5) - 0.5) / fwidth(uv);

    float line = min(grid.x, grid.y);

    // Smoothstep
    float lineFactor = 1.0 - smoothstep(gLineWidth - 1.0, gLineWidth + 1.0, line);

    vec4 finalColor = mix(gColor, gLineColor, lineFactor);

    gAlbedo = finalColor;
}

