//
// Created by radue on 11/3/2024.
//

#pragma once

#include <memory>
#include <unordered_set>

#include <vulkan/vulkan.hpp>

#include "memory/descriptor/pool.h"

struct ImFont;

namespace Core {
    class Window;
    class Runtime;
    class Device;
    class Scheduler;
}

namespace GUI {
    class Layer;

    enum FontType {
        Light = 0,
        Regular = 1,
        Medium = 2,
        Bold = 3,
        Italic = 4,
        Black = 5,
    };


    ImFont* GetFont(FontType type, float size);

    struct CreateInfo {
        const Core::Window& window;
        const Core::Runtime& runtime;
        const Core::Device& device;
        const Core::Scheduler& scheduler;
    };

    inline static std::unique_ptr<Memory::Descriptor::Pool> s_descriptorPool = nullptr;
    void InitDescriptorPool(const Core::Device& device);

    inline std::vector<Layer*> g_layers;
    void AddLayer(Layer* layer);
    void RemoveLayer(Layer* layer);

    void SetupContext(const CreateInfo& createInfo);
    void DestroyContext();

    void Render(const vk::CommandBuffer& commandBuffer);
}

namespace std {
    inline std::string to_string(const GUI::FontType type) {
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
        }
        return "Unknown";
    }
}