//
// Created by radue on 11/3/2024.
//

#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

#include "graphics/framebuffer.h"
#include "imgui.h"
#include "memory/descriptor/pool.h"

namespace Coral::Reef {
	class Popup;
}
namespace Coral::Reef {
    class Manager;
}

struct ImFont;

namespace Coral::Core {
    class Window;
    class Runtime;
    class Device;
    class Scheduler;
}

namespace Coral::Reef {
    class Layer;

    enum class FontType {
        Light,
        Regular,
        Medium,
        Bold,
        Italic,
        Black
    };

    inline Manager* g_manager = nullptr;
    inline Manager& GlobalManager() { return *g_manager; }

    class Manager {
    public:
        void AddFont(std::string path, float size, const ImWchar* ranges);
        void RequestFont(FontType type, float size);
        ImFont* GetFont(FontType type, float size);

        struct CreateInfo {
            const Core::Queue& queue;
            const Graphics::RenderPass& renderPass;

            u32 frameCount;
            vk::Format imageFormat;
            vk::SampleCountFlagBits sampleCount;
        };

        explicit Manager(const CreateInfo& createInfo);
        ~Manager();

        void InitDescriptorPool();

        void AddLayer(Layer* layer);
        void RemoveLayer(Layer* layer);

        void Update(float deltaTime);
        void Render(const Core::CommandBuffer& commandBuffer);

    	void RegisterPopup(const String& name, Popup* popup) {
			m_popups.emplace(name, popup);
		}

    	Popup* GetPopup(const String& name) const {
			if (m_popups.contains(name)) {
				return m_popups.at(name);
			}
			return nullptr;
		}

		void UnregisterPopup(const String& name) {
    		m_popups.erase(name);
    	}

    private:
        std::vector<std::pair<FontType, float>> m_fontRequests {};

    	bool m_frameStarted = false;
        const Core::Queue& m_queue;
        const Graphics::RenderPass& m_renderPass;

        std::vector<Layer*> m_layers;
		std::unordered_map<String, Popup*> m_popups;

        std::unique_ptr<Memory::Descriptor::Pool> m_descriptorPool = nullptr;

        u32 m_frameCount = 0;

        vk::SampleCountFlagBits m_sampleCount;
        vk::Format m_imageFormat;
    };
}