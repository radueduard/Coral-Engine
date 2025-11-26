//
// Created by radue on 3/6/2025.
//

#pragma once

#include <boost/unordered_map.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>

#include "graphics/renderPass.h"
#include "gui/container.h"
#include "gui/manager.h"
#include "gui/viewport.h"

namespace Coral::Core {
    class Frame;
}

namespace Coral::Project {
    class RenderGraph : public Reef::Layer {
    public:
        struct CreateInfo {
            uint32_t frameCount = 2;
            bool guiEnabled = true;
        };

        struct RunNode {
            std::vector<std::string> passes;
            std::vector<std::unique_ptr<Core::CommandBuffer>> commandBuffers {};

            explicit RunNode(std::vector<std::string> passes)
                : passes(std::move(passes)) {}
        };

        explicit RenderGraph(const CreateInfo& createInfo);
        ~RenderGraph() override;

        void Update(float deltaTime) const;
        void Execute(const Core::Frame& frame);
        void Resize(const Math::Vector2<f32>& size, bool inner = false);

        [[nodiscard]] const Memory::Image& OutputImage(uint32_t frameIndex) const;
        [[nodiscard]] vk::Semaphore RenderFinished(uint32_t frameIndex) const;

	protected:
		void OnGUIAttach() override;

	private:
        bool m_guiEnabled = true;
        std::unique_ptr<Reef::Manager> m_guiManager;
        std::unique_ptr<Graphics::RenderPass> m_guiRenderPass;
        std::vector<std::unique_ptr<Core::CommandBuffer>> m_guiCommandBuffers;
        Reef::Container<Reef::Viewport> m_viewport;

        boost::uuids::random_generator_mt19937 m_generator;
        std::unordered_map<vk::QueueFlagBits, std::unique_ptr<Core::Queue>> m_queues;
        uint32_t m_frameCount;
        boost::unordered_map<boost::uuids::uuid, std::vector<Memory::Image*>> m_images;
        std::vector<std::unique_ptr<Memory::Image>> m_imageStorage;
        std::unordered_map<std::string, std::unique_ptr<Graphics::RenderPass>> m_renderPasses;
        std::vector<std::unique_ptr<RunNode>> m_runNodes;

    //  temp:
        std::unique_ptr<Reef::RenderPipelineTemplate> m_pipelineTemplate = nullptr;
        Coral::Graphics::Pipeline::Builder* m_pipelineBuilder = nullptr;

    };
}
