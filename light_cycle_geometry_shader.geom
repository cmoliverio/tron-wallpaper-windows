#version 330 core
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 vViewPos[];
out vec3 fP;
out vec3 fA;
out vec3 fB;

uniform mat4 projection;
uniform float uThickness;

void main()
{
    vec3 a = vViewPos[0];
    vec3 b = vViewPos[1];

    vec3 dir = normalize(b - a);
    vec3 viewDir = normalize(-a);   // camera is at origin in view space
    float dist = max(0.001, -a.z);  // view-space depth
    vec3 right = normalize(cross(dir, viewDir)) * uThickness * dist;

    vec3 quad[4] = vec3[](
        a - right,
        a + right,
        b - right,
        b + right
    );

    for (int i = 0; i < 4; ++i)
    {
        fP = quad[i];
        fA = a;
        fB = b;
        gl_Position = projection * vec4(quad[i], 1.0);
        EmitVertex();
    }

    // geometry shader, before EmitVertex
    gl_Position = vec4(-0.5, -0.5, 0, 1); EmitVertex();
    gl_Position = vec4( 0.5, -0.5, 0, 1); EmitVertex();
    gl_Position = vec4(-0.5,  0.5, 0, 1); EmitVertex();
    gl_Position = vec4( 0.5,  0.5, 0, 1); EmitVertex();
    EndPrimitive();
    return;

    EndPrimitive();
}