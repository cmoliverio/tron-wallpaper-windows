#ifndef LIGHT_CYCLE_HPP
#define LIGHT_CYCLE_HPP

#include <vector>
#include <random>
#include <glm/glm.hpp>

#include "shader.hpp"

enum class Direction {
    Right,
    Up,
    Down,
    Forward,
    Backward
};

inline glm::vec3 direction_vector(Direction d) {
    switch (d) {
        case Direction::Right:    return { 1, 0, 0 };
        case Direction::Up:       return { 0, 1, 0 };
        case Direction::Down:     return { 0,-1, 0 };
        case Direction::Forward:  return { 0, 0, 1 };
        case Direction::Backward: return { 0, 0,-1 };
    }
    return {1,0,0};
}

class LightCycle {
public:
    explicit LightCycle(const glm::vec3& startPos, float speed = 0.5f);

    void change_direction(Direction d);
    void change_direction_random();

    void move(uint64_t elapsed_ms);
    void draw(Shader& shader) const;

    float thickness = 0.15f;

private:
    void push_point_if_needed();

    std::vector<glm::vec3> points;
    glm::vec3 direction;
    glm::vec3 head_position;

    float speed;               // units per second
    float distance_since_last; // for segment spacing

    // OpenGL
    unsigned int vao = 0;
    unsigned int vbo = 0;
};

#endif