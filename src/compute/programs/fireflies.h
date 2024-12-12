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
    struct Particle {
        glm::vec4 origin;
        glm::vec4 position;
        glm::vec4 color;

        static Particle Random(const Graphics::AABB &bounds) {
            Particle particle {};
            particle.origin = { Utils::Random::UniformRealVector(bounds.Min(), bounds.Max()), 0.0f };
            particle.position = { particle.origin.x, 0.0 ,particle.origin.y, Utils::Random::UniformRealValue(1.0f, 2.0f) };
            particle.color = glm::vec4(Utils::Random::Color(), 1.0f);
            return particle;
        }
    };

    struct BezierTrajectory {
        glm::vec4 onCurvePoints[4];
        glm::vec4 controlPoints[8];
    };

    struct CreateInfo {
        const Memory::Buffer &particlesBuffer;
        const Memory::Buffer &trajectoriesBuffer;
        const Memory::Image &heightMap;
    };

    explicit Fireflies(const CreateInfo &createInfo);
    ~Fireflies() override = default;

    void Init() override {}
    void Update() override {}
    void Compute() override;
    void ResetDescriptorSets() override;

private:
    const Memory::Buffer &m_particlesBuffer;
    const Memory::Buffer &m_trajectoriesBuffer;
    const Memory::Image& m_heightMap;

    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
};
