#include "tetrahedron.hpp"

#include <glad/glad.h>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

// ===== Shared geometry (canonical tetrahedron) =====

static constexpr float sqrt2 = 1.41421356237f;
static constexpr float sqrt3 = 1.73205080757f;
static constexpr float sqrt6 = 2.44948974278f;

static const float pyramid_vertices[] = {
     0.0f, 0.0f,  1.0f,
     2.0f * sqrt2 / 3.0f,  0.0f, -1.0f / 3.0f,
    -sqrt2 / 3.0f,  sqrt6 / 3.0f, -1.0f / 3.0f,
    -sqrt2 / 3.0f, -sqrt6 / 3.0f, -1.0f / 3.0f,
};

static const uint32_t pyramid_indices[] = {
    0, 1, 2,
    0, 2, 3,
    0, 3, 1,
    1, 3, 2
};

// ===== Static members =====

bool     Tetrahedron::gpuInitialized = false;
uint32_t Tetrahedron::VAO = 0;
uint32_t Tetrahedron::VBO = 0;
uint32_t Tetrahedron::EBO = 0;

// ===== GPU init (once) =====

void Tetrahedron::initGPU()
{
    if (gpuInitialized)
        return;

    gpuInitialized = true;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(pyramid_vertices),
        pyramid_vertices,
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(pyramid_indices),
        pyramid_indices,
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

// ===== Constructor =====

Tetrahedron::Tetrahedron(const glm::vec3& worldPosition)
{
    initGPU();
    model = glm::translate(glm::mat4(1.0f), worldPosition);
}

// ===== Transform interface =====

void Tetrahedron::translate(const glm::vec3& delta)
{
    model = glm::translate(model, delta);
}

void Tetrahedron::rotate(float radians, const glm::vec3& axis)
{
    model = glm::rotate(model, radians, glm::normalize(axis));
}

void Tetrahedron::scale(const glm::vec3& factors)
{
    model = glm::scale(model, factors);
}

void Tetrahedron::apply(const std::function<void(glm::mat4&)>& transformFn)
{
    transformFn(model);
}

// ===== Rendering =====

void Tetrahedron::draw(Shader& shader) const
{
    shader.use();

    int loc = glGetUniformLocation(shader.ID, "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

    loc = glGetUniformLocation(shader.ID, "number");
    glUniform1ui(loc, 0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Tetrahedron::draw(Shader& shader, int obj_num) const
{
    shader.use();

    int loc = glGetUniformLocation(shader.ID, "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

    loc = glGetUniformLocation(shader.ID, "obj_num");
    glUniform1i(loc, obj_num);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

const glm::mat4& Tetrahedron::getModelMatrix() const
{
    return model;
}