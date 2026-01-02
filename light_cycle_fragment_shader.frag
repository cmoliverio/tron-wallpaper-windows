#version 330 core

out vec4 FragColor;

void main() {
    float glow = 1.0; // later: distance from center
    FragColor = vec4(0.1, 0.8, 1.0, 1.0) * glow;
}