#version 330 core
layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aColor;


// uniform mat4 camera;
uniform mat4 transform;

uniform vec2 viewport_size;

out vec3 theColor;

void main()
{
    // normalize x and y based on the window resolution
    // vec2 x_y_pos = aPos.xy;
    // x_y_pos /= viewport_size.x / viewport_size.y;
    float ratio = viewport_size.x / viewport_size.y;
    // aPos.xy = x_y_pos.xy;

    // mat4 something = camera;

    gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);

    if (ratio > 1.0) {
        // Window is wide: shrink X
        gl_Position.x /= ratio;
    } else {
        // Window is tall: shrink Y
        gl_Position.y *= ratio;
    }

    theColor = vec3(gl_Position.x + gl_Position.y, 
        gl_Position.y + gl_Position.z, 
        gl_Position.z);
}