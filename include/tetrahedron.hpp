#ifndef TETRAHEDRON_HPP
#define TETRAHEDRON_HPP

#include <cstdint>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"

class Tetrahedron {
public:
    // Construct at a world position
    explicit Tetrahedron(const glm::vec3& worldPosition);

    // Transform interface
    void translate(const glm::vec3& delta);
    void rotate(float radians, const glm::vec3& axis);
    void scale(const glm::vec3& factors);

    // Generic transform hook (advanced / optional)
    void apply(const std::function<void(glm::mat4&)>& transformFn);

    // Rendering
    void draw(Shader& shader) const;

    // Access if needed
    const glm::mat4& getModelMatrix() const;

private:
    glm::mat4 model{1.0f};

    // Shared GPU resources
    static bool gpuInitialized;
    static uint32_t VAO;
    static uint32_t VBO;
    static uint32_t EBO;

    static void initGPU();
};

#endif // TETRAHEDRON_HPP