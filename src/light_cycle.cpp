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

// LightCycle::LightCycle(const glm::vec3& startPos, float s)
//     : direction(direction_vector(Direction::Right)),
//       speed(s),
//       distance_since_last(0.0f)
// {
//     // start point
//     points.push_back(startPos);
//     // push again for head
//     points.push_back(startPos);
    
//     // CREATE THE FIRST CAPSULE HERE
//     capsules.emplace_back(thickness * 0.5f);
//     capsules.back().setEndpoints(startPos, startPos);
    
//     glGenVertexArrays(1, &vao);
//     glGenBuffers(1, &vbo);
//     glBindVertexArray(vao);
//     glBindBuffer(GL_ARRAY_BUFFER, vbo);
//     glBufferData(
//         GL_ARRAY_BUFFER,
//         points.size() * sizeof(glm::vec3),
//         points.data(),
//         GL_DYNAMIC_DRAW
//     );
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
//     glBindVertexArray(0);
// }

// LightCycle::LightCycle(const glm::vec3& startPos, float s)
//     : direction(direction_vector(Direction::Right)),
//       speed(s),
//       distance_since_last(0.0f)
// {
//     // start point
//     points.push_back(startPos);

//     // push again for head
//     points.push_back(startPos);

//     glGenVertexArrays(1, &vao);
//     glGenBuffers(1, &vbo);

//     glBindVertexArray(vao);
//     glBindBuffer(GL_ARRAY_BUFFER, vbo);

//     glBufferData(
//         GL_ARRAY_BUFFER,
//         points.size() * sizeof(glm::vec3),
//         points.data(),
//         GL_DYNAMIC_DRAW
//     );

//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

//     glBindVertexArray(0);
// }

void LightCycle::change_direction(Direction d)
{
    glm::vec3 endpoint = points.back();
    points.push_back(endpoint);
    points.push_back(endpoint + (direction * speed));
    
    // Create new capsule for the new segment
    // for(int i = 0; i < capsules.size(); i++)
    // {
    //     Capsule *cap = &capsules[i];
    //     // std::cout << cap->radius << " x" << cap.p1.x << " y" << cap.p1.y 
    //     //     << "  z" << cap.p1.x << std::endl;
    //     // std::cout << cap.radius << " x" << cap.p2.x << " y" << cap.p2.y 
    //     //     << "  z" << cap.p2.x << std::endl;
    //     // std::cout << std::endl;
    // }
    // std::cout << "-------" << std::endl;
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
    
    // Set identity model matrix since capsules are already in world space
    // glm::mat4 identity = glm::mat4(1.0f);
    // int32_t model_loc = glGetUniformLocation(shader.ID, "model");
    // if (model_loc != -1) {
    //     glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(identity));
    // }
    
    // std::cout << "capsules leng" << capsules.size() << std::endl;
    
    for (const auto& c : capsules) {
        c->draw(shader);
    }
}