#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 theColor;

void main()
{
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    theColor = vec3(gl_Position.x, 
        gl_Position.y + aPos.y, 
        gl_Position.z);
}