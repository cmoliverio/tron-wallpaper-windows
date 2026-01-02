#version 330 core
layout (location = 0) in vec3 aPos;

uniform vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vViewPos;

void main()
{
    // view coordinates for geometry shader
    vec4 viewPos = view * model * vec4(aPos, 1.0);
    vViewPos = viewPos.xyz;

    // normal pos
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}