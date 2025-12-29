#version 330 core
flat in int faceID;
out vec4 FragColor;

const vec3 face_colors[4] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 1.0, 0.0)
);

void main()
{
    // FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    // FragColor = vec4(theColor.x, theColor.y, theColor.z, 0.1f);
    FragColor = vec4(face_colors[faceID], 1.0);
} 