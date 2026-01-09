#include "light_cycle.hpp"

#include <glad/glad.h>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

void LightCycle::print_points()
{
    // std::cout << "points (size = " << points.size() << ")" << std::endl;

    // for (const auto& x : points) {
    //     std::cout << x.x << ' '
    //         << x.y << ' '
    //         << x.z << ' '
    //         << std::endl;;
    // }
}

LightCycle::LightCycle(const glm::vec3& startPos, float s)
    : direction(direction_vector(Direction::Right)),
      speed(s),
      distance_since_last(0.0f)
{
    points.push_back(startPos);
    points.push_back(startPos + (direction * speed));
    
    // Create first capsule
    capsules.push_back(
        std::make_unique<Capsule>(
            startPos,
            startPos + (direction * speed),
            thickness * 0.5f
        )
    );
}

void LightCycle::change_direction(Direction d)
{
    glm::vec3 endpoint = points.back();
    points.push_back(endpoint);
    points.push_back(endpoint + (direction * speed));
    
    capsules.push_back(
        std::make_unique<Capsule>(
            endpoint,
            endpoint + (direction * speed),
            thickness * 0.5f
        )
    );
    
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
    glm::vec3 tail = points[points.size() - 2];
    float distance = elapsed_ms * speed;
    head += direction * distance;
    
    // Rebuild the last capsule with new endpoints
    capsules.back()->setEndpoints(tail, head);
}

void LightCycle::draw(Shader& shader) const
{
    shader.use();
    
    for (const auto& c : capsules) {
        c->draw(shader);
    }
}