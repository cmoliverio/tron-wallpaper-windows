#include "capsule.hpp"
#include <iostream>
#include <vector>
#include <cmath>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

Capsule::Capsule(const glm::vec3& start, const glm::vec3& end, float r)
    : p1(start), p2(end), radius(r)
{
    buildCapsuleGeometry(p1, p2);
}

void Capsule::setEndpoints(const glm::vec3& start, const glm::vec3& end)
{
    p1 = start;
    p2 = end;
    buildCapsuleGeometry(p1, p2);
}

void Capsule::buildCapsuleGeometry(const glm::vec3& start, const glm::vec3& end)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Calculate segment direction and length
    glm::vec3 dir = end - start;
    float segmentLength = glm::length(dir);
    
    glm::vec3 axis;
    if (segmentLength < 1e-6f) {
        axis = glm::vec3(0, 0, 1);
        segmentLength = 0.0f;
    } else {
        axis = glm::normalize(dir);
    }

    // Create rotation matrix to align Z-axis with segment direction
    glm::vec3 zAxis(0, 0, 1);
    glm::mat4 rotation = glm::mat4(1.0f);
    
    float cosTheta = glm::dot(zAxis, axis);
    glm::vec3 rotAxis = glm::cross(zAxis, axis);
    
    if (glm::length(rotAxis) > 1e-6f) {
        rotation = glm::rotate(
            glm::mat4(1.0f),
            acos(glm::clamp(cosTheta, -1.0f, 1.0f)),
            glm::normalize(rotAxis)
        );
    } else if (cosTheta < -0.9f) {
        rotation = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(1, 0, 0));
    }

    // Lambda to transform local coordinates to world coordinates
    auto transformPoint = [&](const glm::vec3& localPos) -> glm::vec3 {
        glm::vec4 rotated = rotation * glm::vec4(localPos, 1.0f);
        return start + glm::vec3(rotated);
    };

    auto transformNormal = [&](const glm::vec3& localNormal) -> glm::vec3 {
        glm::vec4 rotated = rotation * glm::vec4(localNormal, 0.0f);
        return glm::normalize(glm::vec3(rotated));
    };

    // --------------------------------------------------
    // Bottom sphere at start point
    // --------------------------------------------------
    uint32_t botBase = (uint32_t)vertices.size();

    for (int stack = 0; stack <= STACKS * 2; ++stack) {
        float v = (float)stack / (STACKS * 2);
        float phi = v * glm::pi<float>() - glm::half_pi<float>();  // -PI/2 to +PI/2

        float y = sin(phi);
        float r = cos(phi);

        for (int slice = 0; slice <= SLICES; ++slice) {
            float u = (float)slice / SLICES;
            float theta = u * glm::two_pi<float>();

            float x = r * cos(theta);
            float z = r * sin(theta);

            glm::vec3 localNormal = glm::normalize(glm::vec3(x, y, z));
            glm::vec3 localPos = localNormal * radius;
            
            glm::vec3 worldPos = start + localPos;  // Sphere centered at start
            glm::vec3 worldNormal = glm::normalize(localNormal);

            vertices.push_back({ worldPos, worldNormal });
        }
    }

    for (int stack = 0; stack < STACKS * 2; ++stack) {
        for (int slice = 0; slice < SLICES; ++slice) {
            uint32_t i0 = botBase + stack * (SLICES + 1) + slice;
            uint32_t i1 = i0 + SLICES + 1;

            indices.insert(indices.end(), {
                i0, i1, i0 + 1,
                i0 + 1, i1, i1 + 1
            });
        }
    }

    // --------------------------------------------------
    // Cylinder from start to end
    // --------------------------------------------------
    uint32_t cylBase = (uint32_t)vertices.size();
    
    for (int i = 0; i <= SLICES; ++i) {
        float theta = (float)i / SLICES * glm::two_pi<float>();
        float x = cos(theta);
        float y = sin(theta);

        glm::vec3 localNormal = glm::vec3(x, y, 0);
        glm::vec3 worldNormal = transformNormal(localNormal);

        // Bottom ring at z=0 (at start point)
        glm::vec3 localPosBot = glm::vec3(radius * x, radius * y, 0.0f);
        glm::vec3 worldPosBot = transformPoint(localPosBot);
        vertices.push_back({ worldPosBot, worldNormal });

        // Top ring at z=segmentLength (at end point)
        glm::vec3 localPosTop = glm::vec3(radius * x, radius * y, segmentLength);
        glm::vec3 worldPosTop = transformPoint(localPosTop);
        vertices.push_back({ worldPosTop, worldNormal });
    }

    for (int i = 0; i < SLICES; ++i) {
        uint32_t i0 = cylBase + i * 2;
        uint32_t i1 = i0 + 1;
        uint32_t i2 = i0 + 2;
        uint32_t i3 = i0 + 3;

        indices.insert(indices.end(), {
            i0, i2, i1,
            i1, i2, i3
        });
    }

    // --------------------------------------------------
    // Top sphere at end point
    // --------------------------------------------------
    uint32_t topBase = (uint32_t)vertices.size();

    for (int stack = 0; stack <= STACKS * 2; ++stack) {
        float v = (float)stack / (STACKS * 2);
        float phi = v * glm::pi<float>() - glm::half_pi<float>();  // -PI/2 to +PI/2

        float y = sin(phi);
        float r = cos(phi);

        for (int slice = 0; slice <= SLICES; ++slice) {
            float u = (float)slice / SLICES;
            float theta = u * glm::two_pi<float>();

            float x = r * cos(theta);
            float z = r * sin(theta);

            glm::vec3 localNormal = glm::normalize(glm::vec3(x, y, z));
            glm::vec3 localPos = localNormal * radius;
            
            glm::vec3 worldPos = end + localPos;  // Sphere centered at end
            glm::vec3 worldNormal = glm::normalize(localNormal);

            vertices.push_back({ worldPos, worldNormal });
        }
    }

    for (int stack = 0; stack < STACKS * 2; ++stack) {
        for (int slice = 0; slice < SLICES; ++slice) {
            uint32_t i0 = topBase + stack * (SLICES + 1) + slice;
            uint32_t i1 = i0 + SLICES + 1;

            indices.insert(indices.end(), {
                i0, i1, i0 + 1,
                i0 + 1, i1, i1 + 1
            });
        }
    }

    indexCount = (uint32_t)indices.size();

    // Delete old buffers if they exist
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    // Upload to GPU
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(uint32_t),
        indices.data(),
        GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glBindVertexArray(0);
}

void Capsule::draw(Shader& shader) const
{
// Since our geometry is already in world space, use identity matrix
    glm::mat4 model = glm::mat4(1.0f);
    int32_t model_loc = glGetUniformLocation(shader.ID, "model");
    if (model_loc != -1) {
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
    }
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}