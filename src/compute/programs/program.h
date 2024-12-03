//
// Created by radue on 11/29/2024.
//

#pragma once
#include <memory>

#include "compute/pipeline.h"
#include "gui/layer.h"
#include "memory/descriptor/setLayout.h"

namespace Compute {

    class Program : public GUI::Layer {
    public:
        Program();
        ~Program() override;

        Program(const Program &) = delete;
        Program &operator=(const Program &) = delete;

        virtual void Init() = 0;
        virtual void Update() = 0;
        virtual void Compute(const vk::CommandBuffer &commandBuffer) = 0;

        void InitUI() override = 0;
        void UpdateUI() override = 0;
        void DrawUI() override = 0;
        void DestroyUI() override = 0;

    protected:
        std::unique_ptr<Memory::Descriptor::SetLayout> m_setLayout;
        std::unique_ptr<Pipeline> m_pipeline;
    };

}
