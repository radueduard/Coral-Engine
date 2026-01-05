//
// Created by radue on 3/6/2025.
//

#pragma once

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>

#include "graphics/dynamicRender.h"
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
			const RenderGraph& renderGraph;
        	RunNode* previousNode = nullptr;
        	std::unique_ptr<RunNode> nextNode = nullptr;

            std::vector<std::unique_ptr<Core::CommandBuffer>> commandBuffers {};

            explicit RunNode(const RenderGraph& renderGraph)
                : renderGraph(renderGraph) {}
			virtual ~RunNode() = default;

        	void ExecuteNode(const Core::Frame& frame, const Core::Queue& queue);

        	virtual void Run(const Core::CommandBuffer& commandBuffer, u32 frameIndex) = 0;
        	virtual std::string LastPassName() const = 0;
        };

    	struct RenderPassRunNode : RunNode {
    		RenderPassRunNode(const RenderGraph& renderGraph, std::vector<std::string> passes)
				:RunNode(renderGraph), passes(passes) {}

    		void Run(const Core::CommandBuffer& commandBuffer, u32 frameIndex) override;
    		std::string LastPassName() const override {
				if (passes.empty()) {
					return "";
				}
				return passes.back();
			}

            std::vector<std::string> passes {};
    	};

    	struct ShadowRunNode : RunNode {
			ShadowRunNode(const RenderGraph& renderGraph, const Graphics::DynamicRender& shadowRender, const Math::Vector2u& shadowMapSize, const u32 cascadeCount)
				:RunNode(renderGraph), shadowRender(shadowRender), shadowMapSize(shadowMapSize), cascadeCount(cascadeCount) {}

    		void Run(const Core::CommandBuffer& commandBuffer, u32 frameIndex) override;

    		std::string LastPassName() const override {
				return "";
			}

    	private:
    		const Graphics::DynamicRender& shadowRender;
    		Math::Vector2u shadowMapSize;
    		u32 cascadeCount;

    		std::vector<std::unique_ptr<Memory::ImageView>> shadowMapViews {};
    		std::map<std::pair<entt::entity, u32>, Memory::ImageView*> cascadeMapViews {};
    	};

        explicit RenderGraph(const CreateInfo& createInfo);
        ~RenderGraph() override;

        void Update(float deltaTime) const;
        void Execute(const Core::Frame& frame) const;
        void Resize(const Math::Vector2<f32>& size, bool inner = false);

        [[nodiscard]] const Memory::Image& OutputImage(uint32_t frameIndex) const;
        [[nodiscard]] vk::Semaphore RenderFinished(uint32_t frameIndex) const;

    	void AddNode(std::unique_ptr<RunNode> node);

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
    	std::unordered_map<std::string, std::unique_ptr<Graphics::DynamicRender>> m_dynamicRenders;

    	std::unique_ptr<RunNode> m_rootNode = nullptr;
    	RunNode* m_lastNode = nullptr;

    //  temp:
        std::unique_ptr<Reef::RenderPipelineTemplate> m_pipelineTemplate;
        Graphics::Pipeline::Builder* m_pipelineBuilder = nullptr;

    };
}
