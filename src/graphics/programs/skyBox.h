//
// Created by radue on 11/29/2024.
//

#pragma once

#include "program.h"

#include "memory/descriptor/set.h"
#include "graphics/objects/cubeMap.h"

class SkyBox final : public Graphics::Program {
public:
    struct CreateInfo {
        std::unique_ptr<Graphics::CubeMap> cubeMap;
    };

    explicit SkyBox(CreateInfo &createInfo);
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
    std::unique_ptr<Graphics::CubeMap> m_cubeMap;
    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
};
