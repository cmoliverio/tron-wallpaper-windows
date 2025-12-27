#version 330 core
out vec4 FragColor;

in vec3 theColor;

void main()
{
    // FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    FragColor = vec4(theColor.x, theColor.y, theColor.z, 0.1f);
} 