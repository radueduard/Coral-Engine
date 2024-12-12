//
// Created by radue on 12/11/2024.
//

#pragma once

#include "program.h"

namespace Memory {
    class Buffer;
}

class DisplayViewFrustums final : public Graphics::Program
{
public:
    struct CreateInfo {
        const Memory::Buffer& frustumsBuffer;
    };

    explicit DisplayViewFrustums(const CreateInfo &createInfo);
    ~DisplayViewFrustums() override = default;

    // Graphics::Program
    void Init() override {}
    void Update(double deltaTime) override {}
    void Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass* renderPass) const override;
    void ResetDescriptorSets() override;

    // GUI
    void OnUIAttach() override {}
    void OnUIUpdate() override {}
    void OnUIRender() override;
    void OnUIDetach() override {}
private:
    bool m_enabled = false;
    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
    const Memory::Buffer& m_frustumsBuffer;
};
