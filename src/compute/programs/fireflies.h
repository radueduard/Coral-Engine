//
// Created by radue on 12/1/2024.
//
#pragma once

#include "program.h"
#include "components/camera.h"
#include "graphics/objects/aabb.h"
#include "graphics/programs/program.h"
#include "memory/buffer.h"
#include "memory/image.h"
#include "utils/random.h"

class Fireflies final : public Compute::Program {
public:
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
        const Memory::Image *heightMap;
        const Memory::Buffer<Particle> &particlesBuffer;
    };

    Fireflies(const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo);
    ~Fireflies() override = default;

    // Compute::Program
    void Init() override;
    void Update() override;
    void Compute(const vk::CommandBuffer &commandBuffer) override;

    // GUI
    void InitUI() override;
    void UpdateUI() override;
    void DrawUI() override;
    void DestroyUI() override;
private:
    const Memory::Buffer<Particle> &m_particlesBuffer;

    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
};
