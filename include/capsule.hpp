#ifndef CAPSULE_HPP
#define CAPSULE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include "shader.hpp"

class Capsule {
public:
    Capsule(const glm::vec3& p1, const glm::vec3& p2, float radius);
    
    // Recreate geometry with new endpoints
    void setEndpoints(const glm::vec3& p1, const glm::vec3& p2);
    
    void draw(Shader& shader) const;
    
    ~Capsule() {
        if (vao) glDeleteVertexArrays(1, &vao);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (ebo) glDeleteBuffers(1, &ebo);
    }

    glm::vec3 p1, p2;
    float radius;

private:
    void buildCapsuleGeometry(const glm::vec3& p1, const glm::vec3& p2);
    
    
    static constexpr int SLICES = 24;
    static constexpr int STACKS = 12;
    
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    unsigned int indexCount = 0;
};

#endif // CAPSULE_HPP