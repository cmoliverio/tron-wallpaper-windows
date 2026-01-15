#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 vNormal;
in vec3 vViewPos;
uniform vec3 uColor;
uniform float uHdrMultiplier;    // NEW: HDR glow intensity
uniform float uEdgeSmoothing;    // NEW: Edge smoothstep range
uniform vec2 uAaMix;             // NEW: AA factor min/max (x=min, y=max)

void main()
{
    // Use the color passed from the application, multiplied for HDR glow
    vec3 color = uColor * uHdrMultiplier; // Multiply by 3 for HDR glow
    
    // Enhanced edge-based anti-aliasing using normal
    // The derivative functions measure how quickly values change across fragments
    vec3 fdx = dFdx(vViewPos);
    vec3 fdy = dFdy(vViewPos);
    vec3 normal = normalize(cross(fdx, fdy));
    
    // Calculate edge factor based on view angle
    // More perpendicular to view = stronger edge = more aliasing
    float edgeFactor = abs(dot(normalize(vViewPos), normal));
    edgeFactor = smoothstep(0.0, uEdgeSmoothing, edgeFactor); // Wider smooth transition for more AA
    
    // Apply stronger anti-aliasing by more aggressively reducing intensity at steep angles
    float aaFactor = mix(uAaMix.x, uAaMix.y, edgeFactor);
    color *= aaFactor;
    
    FragColor = vec4(color, 1.0);
    
    // Extract bright areas based on luminance
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    BrightColor = vec4(color, 1.0);
}