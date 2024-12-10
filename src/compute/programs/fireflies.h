//
// Created by radue on 12/1/2024.
//
#pragma once

#include "program.h"

#include <glm/glm.hpp>

#include "utils/random.h"
#include "graphics/objects/aabb.h"

#include "memory/descriptor/set.h"

namespace Memory {
    class Image;
    class Buffer;
}

class Fireflies final : public Compute::Program {
public:
    struct Frustum {
        glm::vec4 planes[4];
    };

    struct Particle {
        glm::vec4 position;
        glm::vec4 velocity;
        glm::vec4 acceleration;
        glm::vec4 color;

        static Particle Random(const Graphics::AABB &bounds) {
            return {
                .position = glm::vec4(Utils::Random::UniformRealVector(bounds.Min(), bounds.Max()), 1.0f),
                .velocity = glm::vec4(0.0f),
                .acceleration = glm::vec4(0.0f),
                .color = glm::vec4(Utils::Random::Color(), 1.0f)
            };
        }
    };

    struct CreateInfo {
        const Memory::Image &heightMap;
        const Memory::Buffer &particlesBuffer;
    };

    explicit Fireflies(const CreateInfo &createInfo);
    ~Fireflies() override = default;

    void Init() override {}
    void Update() override {}
    void Compute() override;
    void ResetDescriptorSets() override;

private:
    const Memory::Image& m_heightMap;
    const Memory::Buffer &m_particlesBuffer;

    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
};
