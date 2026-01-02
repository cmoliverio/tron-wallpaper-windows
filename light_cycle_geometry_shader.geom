#version 330 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 vViewPos[];

uniform mat4 projection;
uniform float thickness;

void main()
{
    vec3 p0 = vViewPos[0];
    vec3 p1 = vViewPos[1];

    vec3 dir = normalize(p1 - p0);
    vec3 right = normalize(cross(dir, vec3(0,0,1))) * thickness;

    vec4 v0 = projection * vec4(p0 - right, 1.0);
    vec4 v1 = projection * vec4(p0 + right, 1.0);
    vec4 v2 = projection * vec4(p1 - right, 1.0);
    vec4 v3 = projection * vec4(p1 + right, 1.0);

    gl_Position = v0; EmitVertex();
    gl_Position = v1; EmitVertex();
    gl_Position = v2; EmitVertex();
    gl_Position = v3; EmitVertex();
    EndPrimitive();
}