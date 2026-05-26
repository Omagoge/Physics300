#version 460
uniform sampler2D sdfTex;
in vec2 TexCoord;

out vec3 outColor;

void main() {
    float d = texture(sdfTex, TexCoord).r;

    // scale for visibility
    float v = d * 20.0;

    outColor = vec3(v);
}