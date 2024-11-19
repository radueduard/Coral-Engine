//
// Created by radue on 11/8/2024.
//
#pragma once

#include "components/camera.h"
#include "compute/pipeline.h"
#include "memory/buffer.h"
#include "scene/scene.h"

class CalculateMVP {
public:
    CalculateMVP(Core::Device &device, const Memory::Descriptor::Pool &pool);

    void Init(const mgv::Scene &scene) const;
    void Update(const mgv::Scene &scene) const;

    void Compute(const vk::CommandBuffer& commandBuffer) const;

    const Memory::Buffer<glm::mat4>& ModelBuffer() const { return *m_outModelBuffer; }
    const Memory::Buffer<glm::mat4>& MVPBuffer() const { return *m_mvpBuffer; }

private:
    std::unique_ptr<Memory::Descriptor::SetLayout> m_setLayout;
    std::unique_ptr<Compute::Pipeline> m_computeProgram;

    std::unique_ptr<Memory::Buffer<mgv::Camera::Info>> m_cameraBuffer;
    std::unique_ptr<Memory::Buffer<glm::mat4>> m_modelBuffer;
    std::unique_ptr<Memory::Buffer<uint32_t>> m_parentBuffer;
    std::unique_ptr<Memory::Buffer<glm::mat4>> m_outModelBuffer;
    std::unique_ptr<Memory::Buffer<glm::mat4>> m_mvpBuffer;

    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
};
