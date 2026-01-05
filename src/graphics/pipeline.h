//
// Created by radue on 10/17/2024.
//

#pragma once

#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "shader/shader.h"
#include "memory/descriptor/set.h"
#include "objects/mesh.h"

namespace Coral::Shader {
	class Shader;
	enum class Stage : uint32_t;
}
namespace Coral::Memory::Descriptor {
    class SetLayout;
}


namespace Coral::Reef {
	class RenderPipelineTemplate;
}

namespace Coral::Graphics {
    class Pipeline {
    	friend class RenderPass;
    public:
    	class Builder {
            friend class RenderPass;
    		friend class Pipeline;
		public:
    		Builder() = default;
    		virtual ~Builder() = default;
    		Builder(const Builder &) = delete;
    		Builder &operator=(const Builder &) = delete;

    		virtual std::unique_ptr<Pipeline> Build() = 0;

    		bool ShouldRebuild() {
    			if (m_shouldRebuild) {
    				m_shouldRebuild = false;
    				return true;
    			}
    			return false;
    		}

    	protected:
    		bool m_shouldRebuild = true;

    		std::vector<std::unique_ptr<Memory::Descriptor::SetLayout>> m_setLayouts;
    		std::unordered_map<Shader::Stage, const Shader::Shader*> m_shaders;
    		std::vector<vk::PipelineShaderStageCreateInfo> m_stages;
    	};

        class BuilderRenderPass : public Builder {
            friend class Pipeline;
            friend class Reef::RenderPipelineTemplate;
        public:
            explicit BuilderRenderPass(RenderPass&);
            ~BuilderRenderPass() override = default;

            BuilderRenderPass(const BuilderRenderPass &) = delete;
            BuilderRenderPass &operator=(const BuilderRenderPass &) = delete;

			BuilderRenderPass &AddShader(const Shader::Shader* shader);

            BuilderRenderPass &InputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo &);
            BuilderRenderPass &Viewport(const vk::Viewport &);
            BuilderRenderPass &Scissor(const vk::Rect2D &);
            BuilderRenderPass &Rasterizer(const vk::PipelineRasterizationStateCreateInfo &);
            BuilderRenderPass &DepthStencil(const vk::PipelineDepthStencilStateCreateInfo &);

            BuilderRenderPass &DynamicState(const vk::DynamicState &);
            BuilderRenderPass &Tessellation(const vk::PipelineTessellationStateCreateInfo &);

            BuilderRenderPass &Subpass(uint32_t);

        	BuilderRenderPass &RenderFunction(const std::function<void(const Pipeline&, const Core::CommandBuffer&)> &function);

            std::unique_ptr<Pipeline> Build() override;
        private:
			RenderPass &m_renderPass;

            vk::PipelineVertexInputStateCreateInfo m_vertexInputInfo;
            vk::PipelineInputAssemblyStateCreateInfo m_inputAssembly;

            std::vector<vk::Viewport> m_viewports;
            std::vector<vk::Rect2D> m_scissors;
            vk::PipelineViewportStateCreateInfo m_viewportState;

            vk::PipelineRasterizationStateCreateInfo m_rasterizer;
            vk::PipelineDepthStencilStateCreateInfo m_depthStencil;

            std::vector<vk::PipelineColorBlendAttachmentState> m_colorBlendAttachments;
            vk::PipelineColorBlendStateCreateInfo m_colorBlending;
            vk::PipelineMultisampleStateCreateInfo m_multisampling;
            vk::PipelineTessellationStateCreateInfo m_tessellation;

            std::vector <vk::DynamicState> m_dynamicStates = {
                vk::DynamicState::eViewport,
                vk::DynamicState::eScissor,
            };
            vk::PipelineDynamicStateCreateInfo m_dynamicState;

            vk::PipelineLayout m_pipelineLayout;
            uint32_t m_subpass = 0;

        	std::function<void(const Pipeline&, const Core::CommandBuffer&)> m_function = nullptr;
        };

		class BuilderDynamic : public Builder {
			friend class Pipeline;
		public:
			explicit BuilderDynamic(const vk::PipelineRenderingCreateInfo& pipelineRenderingInfo)
				: m_pipelineRenderingInfo(pipelineRenderingInfo) {}
			~BuilderDynamic() override = default;

			BuilderDynamic(const BuilderDynamic &) = delete;
			BuilderDynamic &operator=(const BuilderDynamic &) = delete;

			BuilderDynamic &AddShader(const Shader::Shader* shader);
			BuilderDynamic &InputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo &);
			BuilderDynamic &Rasterizer(const vk::PipelineRasterizationStateCreateInfo &);
			BuilderDynamic &DepthStencil(const vk::PipelineDepthStencilStateCreateInfo &);

			BuilderDynamic &DynamicState(const vk::DynamicState &);
			BuilderDynamic &Tessellation(const vk::PipelineTessellationStateCreateInfo &);
			BuilderDynamic &Multisampling(const vk::PipelineMultisampleStateCreateInfo &);

			BuilderDynamic &RenderFunction(const std::function<void(const Pipeline&, const Core::CommandBuffer&)> &function);

			std::unique_ptr<Pipeline> Build() override;

		private:
			vk::PipelineRenderingCreateInfo m_pipelineRenderingInfo;

			vk::PipelineVertexInputStateCreateInfo m_vertexInputInfo;
			vk::PipelineInputAssemblyStateCreateInfo m_inputAssembly;

			vk::PipelineRasterizationStateCreateInfo m_rasterizer;
			vk::PipelineDepthStencilStateCreateInfo m_depthStencil;
			vk::PipelineViewportStateCreateInfo m_viewportState;

			std::vector<vk::PipelineColorBlendAttachmentState> m_colorBlendAttachments;
			vk::PipelineColorBlendStateCreateInfo m_colorBlending;

			vk::PipelineMultisampleStateCreateInfo m_multisampling = vk::PipelineMultisampleStateCreateInfo()
				.setRasterizationSamples(vk::SampleCountFlagBits::e1)
				.setSampleShadingEnable(vk::False)
				.setMinSampleShading(1.0f)
				.setAlphaToCoverageEnable(vk::False)
				.setAlphaToOneEnable(vk::False);

			vk::PipelineTessellationStateCreateInfo m_tessellation;

			std::vector <vk::DynamicState> m_dynamicStates = {
				// vk::DynamicState::eViewport,
				// vk::DynamicState::eScissor,
			};
			vk::PipelineDynamicStateCreateInfo m_dynamicState;

			vk::PipelineLayout m_pipelineLayout;
        	std::function<void(const Pipeline&, const Core::CommandBuffer&)> m_function = nullptr;
		};

        explicit Pipeline(BuilderRenderPass &);
        explicit Pipeline(BuilderDynamic &);
		~Pipeline();

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        void Bind(const vk::CommandBuffer&) const;
        template<typename T>
        void PushConstants(const vk::CommandBuffer commandBuffer, const vk::ShaderStageFlags stageFlags, const uint32_t offset, const T& data) const {
            commandBuffer.pushConstants(
                m_pipelineLayout,
                stageFlags,
                offset,
                sizeof(T),
                &data);
        }

        void BindDescriptorSet(uint32_t, vk::CommandBuffer, const Memory::Descriptor::Set &) const;
        void BindDescriptorSets(uint32_t, vk::CommandBuffer, const std::vector<Memory::Descriptor::Set> &) const;

        [[nodiscard]] const vk::PipelineLayout &Layout() const { return m_pipelineLayout; }

    	void Render(const Core::CommandBuffer& commandBuffer) const {
			if (m_renderFunction) {
				m_renderFunction(*this, commandBuffer);
			}
		}

        const std::unordered_map<Shader::Stage, const Shader::Shader*>& Shaders() { return m_shaders; }
    private:
        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_pipelineLayout;
        std::vector<std::unique_ptr<Memory::Descriptor::SetLayout>> m_setLayouts;
        std::unordered_map<Shader::Stage, const Shader::Shader*> m_shaders;

    	std::function<void(const Pipeline&, const Core::CommandBuffer&)> m_renderFunction = nullptr;
    };
}
