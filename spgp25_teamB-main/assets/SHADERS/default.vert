#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aTangent; // xyz = tangent, w = handedness

uniform mat4 gMVP;
uniform mat4 gWorld;

uniform vec3 lightColor = vec3(1,1,1);

out vec2 vTexCoord;
out vec3 vNormal;      // world space normal
out vec3 vTangent;     // world space tangent
out float vHandedness; // handedness sign
out vec3 vWorldPos;

out vec3 LightingColor;

void main()
{
    gl_Position = gMVP * vec4(aPos, 1.0);
    vTexCoord = aTexCoord;

    vNormal = (gWorld * vec4(aNormal, 0.0)).xyz;

    // Forward tangent to the fragment shader
    vTangent = (gWorld * vec4(aTangent.xyz, 0.0)).xyz;
    vHandedness = aTangent.w;
    vWorldPos = (gWorld * vec4(aPos, 1.0)).xyz;

    

    // Ambient
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(vNormal);
    vec3 lightDir = /*normalize(lightPos - gl_Position);*/ normalize(vec3(-1,1,1));
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(vec3(0.0) - gl_Position.xyz);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    LightingColor = ambient + diffuse + specular;
}
