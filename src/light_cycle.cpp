#include "light_cycle.hpp"

#include <glad/glad.h>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

void LightCycle::print_points()
{
    std::cout << "points (size = " << points.size() << ")" << std::endl;

    for (const auto& x : points) {
        std::cout << x.x << ' '
            << x.y << ' '
            << x.z << ' '
            << std::endl;;
    }
}

LightCycle::LightCycle(const glm::vec3& startPos, float s)
    : direction(direction_vector(Direction::Right)),
      speed(s),
      distance_since_last(0.0f)
{
    // start point
    points.push_back(startPos);

    // push again for head
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
    glm::vec3 strip_endpoint = points.back();

    // push once to end current line
    points.push_back(strip_endpoint);

    // push again to start next line
    points.push_back(strip_endpoint);

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
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(points.size()));
    glBindVertexArray(0);
}