#version 330 core
out vec4 FragColor;

in vec3 theColor;

void main()
{
    // FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    // FragColor = vertexColor;
    FragColor = vec4(theColor, 0.1f);
} 