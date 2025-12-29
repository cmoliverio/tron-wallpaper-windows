#version 330 core
flat in int faceID;
flat in int objectID;
out vec4 FragColor;

float hash1(int n)
{
    return fract(sin(float(n) * 12.9898) * 43758.5453);
}

vec3 randomColor(int obj, int face)
{
    int seed = obj * 31 + face * 7;
    return vec3(
        hash1(seed + 0),
        hash1(seed + 1),
        hash1(seed + 2)
    );
}

void main()
{
    vec3 color = randomColor(objectID, faceID);
    FragColor = vec4(color, 1.0);
}