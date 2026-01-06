// #version 330 core
// #extension GL_ARB_conservative_depth : enable
// in vec3 fP;
// in vec3 fA;
// in vec3 fB;

// out vec4 FragColor;

// uniform float uThickness;
// uniform vec4 color;
// uniform mat4 projection;

// vec3 capsuleNormal(vec3 p, vec3 a, vec3 b)
// {
//     vec3 pa = p - a;
//     vec3 ba = b - a;
//     float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
//     vec3 closest = a + ba * h;
//     return normalize(p - closest);
// }

// float sdCapsule(vec3 p, vec3 a, vec3 b, float r)
// {
//     vec3 pa = p - a;
//     vec3 ba = b - a;
//     float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
//     return length(pa - ba * h) - r;
// }

// void main()
// {
//     float d = sdCapsule(fP, fA, fB, uThickness);
//     if (d > 0.0)
//         discard;

//     // Move point onto capsule surface
//     vec3 n = capsuleNormal(fP, fA, fB);
//     vec3 surfacePos = fP - n * d;

//     // Recompute depth
//     vec4 clip = projection * vec4(surfacePos, 1.0);
//     float depth = clip.z / clip.w;
//     gl_FragDepth = depth * 0.5 + 0.5;

//     FragColor = vec4(0.5, 1.0, 1.0, 1.0);
// }

#version 330 core

in vec3 vNormal;
in vec3 vViewPos;

uniform vec3 uColor;
uniform vec3 uLightPos;   // in view space
uniform float uAmbient = 0.2;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightPos - vViewPos);
    vec3 V = normalize(-vViewPos);

    // Diffuse
    float diff = max(dot(N, L), 0.0);

    // Specular (Blinn)
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0);

    vec3 color =
        uAmbient * uColor +
        diff * uColor +
        spec * vec3(1.0);

    FragColor = vec4(color, 1.0);
}