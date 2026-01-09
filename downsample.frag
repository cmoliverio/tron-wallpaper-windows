#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D srcTexture;
uniform vec2 srcResolution;

void main()
{
    vec2 texelSize = 1.0 / srcResolution;
    float x = texelSize.x;
    float y = texelSize.y;
    
    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    
    vec3 a = texture(srcTexture, vec2(TexCoords.x - 2*x, TexCoords.y + 2*y)).rgb;
    vec3 b = texture(srcTexture, vec2(TexCoords.x,       TexCoords.y + 2*y)).rgb;
    vec3 c = texture(srcTexture, vec2(TexCoords.x + 2*x, TexCoords.y + 2*y)).rgb;
    
    vec3 d = texture(srcTexture, vec2(TexCoords.x - 2*x, TexCoords.y)).rgb;
    vec3 e = texture(srcTexture, vec2(TexCoords.x,       TexCoords.y)).rgb;
    vec3 f = texture(srcTexture, vec2(TexCoords.x + 2*x, TexCoords.y)).rgb;
    
    vec3 g = texture(srcTexture, vec2(TexCoords.x - 2*x, TexCoords.y - 2*y)).rgb;
    vec3 h = texture(srcTexture, vec2(TexCoords.x,       TexCoords.y - 2*y)).rgb;
    vec3 i = texture(srcTexture, vec2(TexCoords.x + 2*x, TexCoords.y - 2*y)).rgb;
    
    vec3 j = texture(srcTexture, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 k = texture(srcTexture, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;
    vec3 l = texture(srcTexture, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 m = texture(srcTexture, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;
    
    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This is a 13 tap downsampling filter
    
    vec3 downsample = e * 0.125;
    downsample += (a + c + g + i) * 0.03125;
    downsample += (b + d + f + h) * 0.0625;
    downsample += (j + k + l + m) * 0.125;
    
    FragColor = vec4(downsample, 1.0);
}