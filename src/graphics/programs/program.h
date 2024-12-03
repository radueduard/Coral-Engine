//
// Created by radue on 11/15/2024.
//

#pragma once
#include <memory>

#include "graphics/pipeline.h"
#include "gui/layer.h"
#include "memory/descriptor/setLayout.h"

namespace Graphics {
    class RenderPass;
    class Program : public GUI::Layer {
    public:
        explicit Program(RenderPass& renderPass, uint32_t subpassIndex = 0);
        ~Program() override;

        virtual void Init() = 0;
        virtual void Update(double deltaTime) = 0;
        virtual void Draw(const vk::CommandBuffer& commandBuffer, bool reflected = false) = 0;
    protected:
        RenderPass& m_renderPass;
        uint32_t m_subpassIndex = 0;
        std::unique_ptr<Memory::Descriptor::SetLayout> m_setLayout;
        // TODO move
        std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
        std::unique_ptr<Pipeline> m_pipeline;
    };

}
