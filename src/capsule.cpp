#include "capsule.hpp"
#include <iostream>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

Capsule::Capsule(float r, float initialLength)
    : radius(r), length(initialLength), p1(0.0f), p2(0.0f, 0.0f, initialLength)
{
    buildUnitCapsule();
    updateModelMatrix();
}

void Capsule::buildUnitCapsule()
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    const float radius = 0.5f;
    const float halfLen = 0.5f;

    // Cylinder
    for (int i = 0; i <= SLICES; ++i) {
        float theta = (float)i / SLICES * glm::two_pi<float>();
        float x = cos(theta);
        float y = sin(theta);

        glm::vec3 normal = { x, y, 0 };

        vertices.push_back({ { radius * x, radius * y, -halfLen }, normal });
        vertices.push_back({ { radius * x, radius * y,  halfLen }, normal });
    }

    uint32_t base = 0;
    for (int i = 0; i < SLICES; ++i) {
        uint32_t i0 = base + i * 2;
        uint32_t i1 = i0 + 1;
        uint32_t i2 = i0 + 2;
        uint32_t i3 = i0 + 3;

        indices.insert(indices.end(), {
            i0, i2, i1,
            i1, i2, i3
        });
    }

    // Top hemisphere (+Z)
    uint32_t topBase = (uint32_t)vertices.size();

    for (int stack = 0; stack <= STACKS; ++stack) {
        float v = (float)stack / STACKS;
        float phi = v * glm::half_pi<float>();

        float z = sin(phi);
        float r = cos(phi);

        for (int slice = 0; slice <= SLICES; ++slice) {
            float u = (float)slice / SLICES;
            float theta = u * glm::two_pi<float>();

            float x = r * cos(theta);
            float y = r * sin(theta);

            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            glm::vec3 pos = normal * radius + glm::vec3(0, 0, halfLen);

            vertices.push_back({ pos, normal });
        }
    }

    for (int stack = 0; stack < STACKS; ++stack) {
        for (int slice = 0; slice < SLICES; ++slice) {
            uint32_t i0 = topBase + stack * (SLICES + 1) + slice;
            uint32_t i1 = i0 + SLICES + 1;

            indices.insert(indices.end(), {
                i0, i1, i0 + 1,
                i0 + 1, i1, i1 + 1
            });
        }
    }

    // Bottom hemisphere (-Z)
    uint32_t botBase = (uint32_t)vertices.size();

    for (int stack = 0; stack <= STACKS; ++stack) {
        float v = (float)stack / STACKS;
        float phi = v * glm::half_pi<float>();

        float z = -sin(phi);
        float r = cos(phi);

        for (int slice = 0; slice <= SLICES; ++slice) {
            float u = (float)slice / SLICES;
            float theta = u * glm::two_pi<float>();

            float x = r * cos(theta);
            float y = r * sin(theta);

            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            glm::vec3 pos = normal * radius - glm::vec3(0, 0, halfLen);

            vertices.push_back({ pos, normal });
        }
    }

    for (int stack = 0; stack < STACKS; ++stack) {
        for (int slice = 0; slice < SLICES; ++slice) {
            uint32_t i0 = botBase + stack * (SLICES + 1) + slice;
            uint32_t i1 = i0 + SLICES + 1;

            indices.insert(indices.end(), {
                i0 + 1, i1, i0,
                i1 + 1, i1, i0 + 1
            });
        }
    }

    indexCount = (uint32_t)indices.size();

    // Upload to GPU
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(uint32_t),
        indices.data(),
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glBindVertexArray(0);
}

void Capsule::updateModelMatrix()
{
    glm::vec3 dir = p2 - p1;
    length = glm::length(dir);

    if (length < 1e-6f) {
        model = glm::translate(glm::mat4(1.0f), p1);
        return;
    }

    glm::vec3 axis = glm::normalize(dir);
    glm::vec3 zAxis = {0, 0, 1};

    // rotation
    float cosTheta = glm::dot(zAxis, axis);
    glm::vec3 rotAxis = glm::cross(zAxis, axis);

    glm::mat4 rotation = glm::mat4(1.0f);
    if (glm::length(rotAxis) > 1e-6f) {
        rotation = glm::rotate(
            glm::mat4(1.0f),
            acos(cosTheta),
            glm::normalize(rotAxis)
        );
    } else if (cosTheta < -0.9f) {
        // Handle 180-degree case (pointing backwards)
        rotation = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(1, 0, 0));
    }

    // scale: radius in X/Y, length in Z
    glm::mat4 scale = glm::scale(
        glm::mat4(1.0f),
        { radius * 2.0f, radius * 2.0f, length }
    );

    // translate to midpoint
    glm::vec3 center = (p1 + p2) * 0.5f;
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), center);

    model = translation * rotation * scale;
}

void Capsule::setEndpoints(const glm::vec3& a, const glm::vec3& b)
{
    p1 = a;
    p2 = b;
    updateModelMatrix();
}

void Capsule::draw(Shader& shader) const
{
    int32_t model_loc = glGetUniformLocation(shader.ID, "model");
    if (model_loc != -1) {
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
    }

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}