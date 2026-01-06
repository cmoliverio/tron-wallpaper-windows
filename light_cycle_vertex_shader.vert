#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vNormal;
out vec3 vViewPos;

void main()
{
    // World position
    vec4 worldPos = model * vec4(aPos, 1.0);

    // View space position (for lighting)
    vec4 viewPos = view * worldPos;
    vViewPos = viewPos.xyz;

    // Correct normal transform
    vNormal = mat3(transpose(inverse(view * model))) * aNormal;

    gl_Position = projection * viewPos;
}