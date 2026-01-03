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
    
    // Vector from line midpoint to camera (at origin in view space)
    vec3 midpoint = (a + b) * 0.5;
    vec3 toCamera = normalize(-midpoint);
    
    // Create perpendicular to both the line and view direction
    vec3 right = normalize(cross(dir, toCamera));
    
    // Handle parallel case
    float rightLen = length(cross(dir, toCamera));
    if (rightLen < 0.001) {
        right = vec3(1.0, 0.0, 0.0);
        if (abs(dot(dir, right)) > 0.9) {
            right = vec3(0.0, 1.0, 0.0);
        }
        right = normalize(cross(dir, right));
    }
    
    // Make quad much wider to encompass full capsule
    float quadWidth = uThickness * 3.0;
    vec3 offset = right * quadWidth;
    
    // Extend along line for spherical caps
    vec3 extend = dir * uThickness * 1.5;
    
    vec3 quad[4] = vec3[](
        a - offset - extend,
        a + offset - extend,
        b - offset + extend,
        b + offset + extend
    );
    
    for (int i = 0; i < 4; ++i)
    {
        fP = quad[i];
        fA = a;
        fB = b;
        gl_Position = projection * vec4(quad[i], 1.0);
        EmitVertex();
    }
    EndPrimitive();
}