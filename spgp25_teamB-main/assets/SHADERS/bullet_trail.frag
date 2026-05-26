#version 460 core
in  vec4 vColor;
out vec4 FragColor;
void main()
{
    if (vColor.a < 0.004)
        discard;
    FragColor = vColor;
}
