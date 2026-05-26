#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 3) in vec4 aColor;
uniform mat4 u_ViewProj;
out vec4 vColor;
void main()
{
    vColor      = aColor;
    gl_Position = u_ViewProj * vec4(aPos, 1.0);
}
