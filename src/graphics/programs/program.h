//
// Created by radue on 11/15/2024.
//

#pragma once
#include <memory>
#include <unordered_map>

#include "gui/layer.h"
#include "memory/descriptor/setLayout.h"
#include "graphics/pipeline.h"

namespace Graphics {
    class RenderPass;
}

namespace Graphics {
    class Program : public GUI::Layer {
    public:
        explicit Program(const std::vector<RenderPass*>& renderPasses);
        ~Program() override;

        virtual void Init() = 0;
        virtual void Update(double deltaTime) = 0;
        virtual void Draw(const vk::CommandBuffer& commandBuffer, const RenderPass* renderPass) const = 0;
        virtual void ResetDescriptorSets() = 0;
    protected:
        std::unique_ptr<Memory::Descriptor::SetLayout> m_setLayout;

        std::vector<RenderPass*> m_renderPasses;
        std::unordered_map<const RenderPass*, std::unique_ptr<Pipeline>> m_pipelines;
    };
}
