//
// Created by radue on 11/3/2024.
//

#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

#include "imgui.h"
#include "graphics/framebuffer.h"
#include "memory/descriptor/pool.h"

namespace GUI {
    class Manager;
}

struct ImFont;

namespace Core {
    class Window;
    class Runtime;
    class Device;
    class Scheduler;
}

namespace GUI {
    class Layer;

    enum class FontType {
        Light,
        Regular,
        Medium,
        Bold,
        Italic,
        Black,
        Count
    };

    inline Manager* g_manager = nullptr;
    inline Manager& GlobalManager() { return *g_manager; }

    class Manager {
    public:
        static void AddFont(std::string path, float size, const ImWchar* ranges);
        static ImFont* GetFont(FontType type, float size);

        struct CreateInfo {
            const Core::Window& window;
            const Core::Runtime& runtime;

            const Core::Queue& queue;
            const Graphics::RenderPass& renderPass;

            uint32_t frameCount;
            vk::Format imageFormat;
            vk::SampleCountFlagBits sampleCount;
        };

        explicit Manager(const CreateInfo& createInfo);
        ~Manager();

        void InitDescriptorPool();

        void AddLayer(Layer* layer);
        void RemoveLayer(Layer* layer);

        void Update(float deltaTime) const;
        void Render(const Core::CommandBuffer& commandBuffer) const;

    private:
        const Core::Queue& m_queue;
        const Graphics::RenderPass& m_renderPass;

        std::vector<Layer*> m_layers;
        std::unique_ptr<Memory::Descriptor::Pool> m_descriptorPool = nullptr;

        uint32_t m_frameCount = 0;

        vk::SampleCountFlagBits m_sampleCount;
        vk::Format m_imageFormat;
    };
}

namespace std {
    inline string to_string(const GUI::FontType& type) {
        switch (type) {
            case GUI::FontType::Light:
                return "Light";
            case GUI::FontType::Regular:
                return "Regular";
            case GUI::FontType::Medium:
                return "Medium";
            case GUI::FontType::Bold:
                return "Bold";
            case GUI::FontType::Italic:
                return "Italic";
            case GUI::FontType::Black:
                return "Black";
            default:
                return "Unknown";
        }
    }
}