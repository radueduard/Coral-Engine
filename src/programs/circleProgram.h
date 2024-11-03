//
// Created by radue on 10/24/2024.
//

#pragma once
#include "core/device.h"
#include "extensions/meshShader.h"
#include "graphics/pipeline.h"
#include "graphics/renderer.h"
#include "memory/buffer.h"

class CircleProgram {
public:
    CircleProgram(const Core::Device &device, const Graphics::Renderer& renderer, const uint32_t instanceCount);

    void Init() const;

    void Update() const;

    void Draw(const vk::CommandBuffer& commandBuffer) const;

private:
    uint32_t m_instanceCount;
    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
    std::unique_ptr<Memory::Descriptor::SetLayout> m_setLayout;
    std::unique_ptr<Memory::Buffer<glm::vec4>> m_vertexColorBuffer;
    std::unique_ptr<Graphics::Pipeline> m_graphicsProgram;
};
