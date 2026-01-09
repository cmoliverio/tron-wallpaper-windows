#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;
uniform float bloomStrength;

// Simple Reinhard tone mapping
vec3 toneMap(vec3 color) {
    return color / (color + vec3(1.0));
}

// Gamma correction
vec3 gammaCorrect(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

void main()
{
    vec3 hdrColor = texture(sceneTexture, TexCoords).rgb;
    vec3 bloomColor = texture(bloomTexture, TexCoords).rgb;
    
    // Additive blending
    hdrColor += bloomColor * bloomStrength;
    
    // Tone mapping
    vec3 result = toneMap(hdrColor);
    
    // Gamma correction
    result = gammaCorrect(result);
    
    FragColor = vec4(result, 1.0);
}