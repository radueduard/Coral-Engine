//
// Created by radue on 11/29/2024.
//

#pragma once

#include "program.h"

namespace Graphics {
    class CubeMap;
}

namespace Memory::Descriptor {
    class Set;
}

class SkyBox final : public Graphics::Program {
public:
    struct CreateInfo {
        const Graphics::CubeMap &cubeMap;
    };

    explicit SkyBox(const CreateInfo &createInfo);

    ~SkyBox() override = default;

    void Init() override {}
    void Update(double deltaTime) override {}
    void Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass* renderPass) const override;
    void ResetDescriptorSets() override;

    void OnUIAttach() override {}
    void OnUIUpdate() override {}
    void OnUIRender() override {}
    void OnUIReset() override {}
    void OnUIDetach() override {}
private:
    const Graphics::CubeMap &m_cubeMap;

    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
};
