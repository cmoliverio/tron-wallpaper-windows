#include "light_cycle.hpp"

#include <glad/glad.h>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

LightCycle::LightCycle(const glm::vec3& startPos, float s)
    : direction(direction_vector(Direction::Right)),
      speed(s),
      distance_since_last(0.0f)
{
    points.push_back(startPos);
    points.push_back(startPos);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        points.size() * sizeof(glm::vec3),
        points.data(),
        GL_DYNAMIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}

void LightCycle::change_direction(Direction d) 
{
    // save the back point (head) and change direction
    points.push_back(points.back());

    direction = direction_vector(d);
}

void LightCycle::change_direction_random() 
{
    static std::mt19937 rng{ std::random_device{}() };
    static std::uniform_int_distribution<int> dist(0, 4);
    change_direction(static_cast<Direction>(dist(rng)));
}

void LightCycle::move(uint64_t elapsed_ms)
{
    glm::vec3& head = points.back();

    float distance = elapsed_ms * speed;

    head += direction * distance;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        points.size() * sizeof(glm::vec3),
        points.data(),
        GL_DYNAMIC_DRAW
    );
}

void LightCycle::draw(Shader& shader) const
{
    shader.use();
    shader.setFloat("uThickness", thickness);

    glBindVertexArray(vao);
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(points.size()));
    glBindVertexArray(0);
}