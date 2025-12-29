#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform int obj_num;

flat out int faceID;
flat out int objectID;

void main()
{
    faceID = gl_PrimitiveIDIn;
    objectID = obj_num;

    for (int i = 0; i < 3; ++i)
    {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}