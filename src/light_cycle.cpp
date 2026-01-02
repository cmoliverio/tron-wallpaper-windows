#include "light_cycle.hpp"

#include <glad/glad.h>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

LightCycle::LightCycle(const glm::vec3& startPos, float s)
    : direction(direction_vector(Direction::Right)),
      head_position(startPos),
      speed(s),
      distance_since_last(0.0f)
{
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

void LightCycle::change_direction(Direction d) {
    direction = direction_vector(d);
    distance_since_last = 0.0f;

    // force a hard corner
    points.push_back(head_position);
}

void LightCycle::change_direction_random() {
    static std::mt19937 rng{ std::random_device{}() };
    static std::uniform_int_distribution<int> dist(0, 4);
    change_direction(static_cast<Direction>(dist(rng)));
}

void LightCycle::move(uint64_t elapsed_ms)
{
    float dt = elapsed_ms * 0.001f;
    float step = speed * dt;

    head_position += direction * step;
    distance_since_last += step;

    constexpr float segment_length = 0.25f;

    if (distance_since_last >= segment_length) {
        points.push_back(head_position);
        distance_since_last = 0.0f;

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            points.size() * sizeof(glm::vec3),
            points.data(),
            GL_DYNAMIC_DRAW
        );
    }
}

void LightCycle::draw(Shader& shader) const
{
    shader.use();
    shader.setFloat("uThickness", thickness);

    // int loc = glGetUniformLocation(shader.ID, "model");
    // glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

    glm::mat4 model(1.0f); // identity
    int loc = glGetUniformLocation(shader.ID, "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(vao);
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(points.size()));
    glBindVertexArray(0);
}