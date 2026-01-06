#ifndef CAPSULE_HPP
#define CAPSULE_HPP

// graphics library mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include "shader.hpp"

class Capsule {
public:
    Capsule(float radius, float initialLength = 0.0f);

    // Set endpoints (this is how it grows)
    void setEndpoints(const glm::vec3& p1, const glm::vec3& p2);

    void draw(Shader& shader) const;

private:
    void buildUnitCapsule();   // build once
    void updateModelMatrix();  // cheap, per-frame

private:
    glm::vec3 p1, p2;
    float radius;
    float length;

    glm::mat4 model{1.0f};

    static constexpr int SLICES = 24;   // around
    static constexpr int STACKS = 12;   // per hemisphere

    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    unsigned int indexCount = 0;
};

#endif // CAPSULE_HPP