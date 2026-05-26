#version 460
out float outMask;

uniform sampler2D Texture;
in vec2 vTexCoord;

void main() {
    if(texture(Texture, vTexCoord).a < 0.999f)
      discard;
    
    outMask = 1.0;
}