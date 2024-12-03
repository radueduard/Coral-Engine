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
    struct CreateInfo {
        uint32_t count = 1024;
        Graphics::AABB bounds;
        const mgv::Camera &camera;
        const Memory::Image *heightMap;
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

    Fireflies(const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo);
    ~Fireflies() override = default;

    [[nodiscard]] const Memory::Buffer<Particle>& ParticlesBuffer() const { return *m_particlesBuffer; }
    [[nodiscard]] uint32_t Count() const { return m_count; }

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
    uint32_t m_count;
    Graphics::AABB m_bounds;

    std::unique_ptr<Memory::Buffer<Particle>> m_particlesBuffer;
    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
};
