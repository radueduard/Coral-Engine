//
// Created by radue on 10/14/2024.
//
#pragma once

#include <functional>
#include <vulkan/vulkan.hpp>

#include "../core/window.h"
#include "../core/device.h"
#include "memory/descriptor/pool.h"
#include "swapChain.h"

namespace Graphics {
    class Renderer {
    public:
        Renderer(const Core::Window &window, const Core::Device &device);
        ~Renderer() = default;

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        bool BeginFrame();
        void Draw(const std::function<void(vk::CommandBuffer)> &drawFunc) const;
        bool EndFrame();

        [[nodiscard]] const SwapChain &SwapChain() const { return *m_swapChain; }
        [[nodiscard]] const Memory::Descriptor::Pool &DescriptorPool() const { return *m_descriptorPool; }
        [[nodiscard]] bool IsFrameStarted() const { return m_frameStarted; }

    private:
        const Core::Window &m_window;
        const Core::Device &m_device;
        std::unique_ptr<Graphics::SwapChain> m_swapChain;

        bool m_frameStarted = false;

        void CreateDescriptorPool();
        std::unique_ptr<Memory::Descriptor::Pool> m_descriptorPool;
    };
}
