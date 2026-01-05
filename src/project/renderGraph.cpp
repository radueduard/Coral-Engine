//
// Created by radue on 3/6/2025.
//
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "renderGraph.h"

#include <queue>
#include <boost/uuid/nil_generator.hpp>

#include "core/scheduler.h"
#include "ecs/entity.h"
#include "graphics/pipeline.h"
#include "graphics/renderPass.h"
#include "gui/container.h"
#include "gui/elements/popup.h"
#include "gui/templates/renderPipelineTemplate.h"
#include "gui/viewport.h"
#include "shader/manager.h"

#include <entt/entity/registry.hpp>


namespace Coral::Project {
	void RenderGraph::ShadowRunNode::Run(const Core::CommandBuffer& commandBuffer, const u32 frameIndex) const {
		ECS::SceneManager::Get().Registry().group(entt::get<ECS::Entity*, ECS::Light>).each(
			[&](ECS::Entity* entity, const ECS::Light& light) {
				if (light.CastsShadows()) {
					for (u32 i = 0; i < cascadeCount; i++) {
						auto view = Memory::ImageView::Builder(light.ShadowMap(frameIndex))
							.ViewType(vk::ImageViewType::e2D)
							.BaseArrayLayer(i)
							.LayerCount(1)
							.Build();
						shadowRender.Render(commandBuffer, vk::RenderingInfo()
							.setLayerCount(1)
							.setViewMask(0)
							.setRenderArea(vk::Rect2D()
								.setOffset({ 0, 0 })
								.setExtent({ 2048, 2048 }))
							.setPDepthAttachment(&vk::RenderingAttachmentInfo()
								.setImageView(**view)
								.setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
								.setLoadOp(vk::AttachmentLoadOp::eClear)
								.setStoreOp(vk::AttachmentStoreOp::eStore)
								.setClearValue(vk::ClearValue().setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0)))
							)
							.setPStencilAttachment(nullptr)
						);
					}
				}
			}
		);
	}


	void RenderGraph::RunNode::ExecuteNode(const Core::Frame& frame, const Core::Queue& queue) const {
		const auto& commandBuffer = *commandBuffers[frame.ImageIndex()];

		commandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
		commandBuffer->begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
		Run(commandBuffer, frame.ImageIndex());
		commandBuffer->end();

		const auto commandBuffers = std::array { *commandBuffer };

		std::vector<vk::Semaphore> waitSemaphores;
		if (previousNode != nullptr) {
			const auto& previousCommandBuffer = *previousNode->commandBuffers[frame.ImageIndex()];
			waitSemaphores.emplace_back(previousCommandBuffer.SignalSemaphore());
		} else {
			waitSemaphores.emplace_back(frame.ImageAvailable());
		}

		constexpr auto destMask = vk::PipelineStageFlags()
			| vk::PipelineStageFlagBits::eColorAttachmentOutput;

		std::vector signalSemaphores = {
			commandBuffer.SignalSemaphore()
		};

		if (!renderGraph.m_guiEnabled && nextNode == nullptr) {
			signalSemaphores.emplace_back(frame.ReadyToPresent());
		}

		const auto submitInfo = vk::SubmitInfo()
			.setCommandBuffers(commandBuffers)
			.setWaitSemaphores(waitSemaphores)
			.setSignalSemaphores(signalSemaphores)
			.setWaitDstStageMask(destMask);

		try {
			queue->submit(submitInfo);
		} catch (const vk::OutOfDateKHRError&) {
			// Recreate framebuffers
		}
	}
	void RenderGraph::RenderPassRunNode::Run(const Core::CommandBuffer& commandBuffer, const u32 frameIndex) const {
		for (const auto& renderPass : passes) {
			renderGraph.m_renderPasses.at(renderPass)->Begin(commandBuffer, frameIndex);
			renderGraph.m_renderPasses.at(renderPass)->Draw(commandBuffer);
			renderGraph.m_renderPasses.at(renderPass)->End(commandBuffer);
		}
	}

	RenderGraph::RenderGraph(const CreateInfo& createInfo)
		: m_guiEnabled(createInfo.guiEnabled), m_frameCount(createInfo.frameCount) {
		m_generator = boost::uuids::random_generator_mt19937();

		m_pipelineTemplate = std::make_unique<Reef::RenderPipelineTemplate>();

		auto windowSize = Core::Window::Get().Extent();
		Math::Vector3u extent = { static_cast<u32>(windowSize.width), static_cast<u32>(windowSize.height), 1u };

		auto idGui = boost::uuids::nil_uuid();
		if (m_guiEnabled) {
			idGui = m_generator();
			m_images.emplace(idGui, std::vector<Memory::Image*>());
		}

		auto idDepth = m_generator();
		auto idColor = m_generator();
		auto idColorResolve = m_generator();

		m_images.emplace(idDepth, std::vector<Memory::Image*>());
		m_images.emplace(idColor, std::vector<Memory::Image*>());
		m_images.emplace(idColorResolve, std::vector<Memory::Image*>());

		for (uint32_t i = 0; i < m_frameCount; i++) {
			Memory::Image* depthImage = m_imageStorage.emplace_back(
				Memory::Image::Builder()
					.Format(vk::Format::eD32SfloatS8Uint)
					.Extent(extent)
					.UsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment)
					.SampleCount(vk::SampleCountFlagBits::e2)
					.InitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
					.Build()).get();
			m_images.at(idDepth).emplace_back(depthImage);

			Memory::Image* colorImage = m_imageStorage.emplace_back(
				Memory::Image::Builder()
					.Format(vk::Format::eB8G8R8A8Unorm)
					.Extent(extent)
					.UsageFlags(vk::ImageUsageFlagBits::eColorAttachment)
					.UsageFlags(vk::ImageUsageFlagBits::eSampled)
					.UsageFlags(vk::ImageUsageFlagBits::eTransferSrc)
					.SampleCount(vk::SampleCountFlagBits::e2)
					.InitialLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.Build()).get();
			m_images.at(idColor).emplace_back(colorImage);

			Memory::Image* colorResolveImage = m_imageStorage.emplace_back(
				Memory::Image::Builder()
					.Format(vk::Format::eB8G8R8A8Unorm)
					.Extent(extent)
					.UsageFlags(vk::ImageUsageFlagBits::eColorAttachment)
					.UsageFlags(vk::ImageUsageFlagBits::eSampled)
					.UsageFlags(vk::ImageUsageFlagBits::eTransferSrc).SampleCount(vk::SampleCountFlagBits::e1)
					.InitialLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.Build()).get();
			m_images.at(idColorResolve).emplace_back(colorResolveImage);

			if (m_guiEnabled) {
				Memory::Image* guiImage = m_imageStorage.emplace_back(
					Memory::Image::Builder()
						.Format(vk::Format::eB8G8R8A8Unorm)
						.Extent( Math::Vector2u { 1920u, 1080u } )
						.UsageFlags(vk::ImageUsageFlagBits::eColorAttachment)
						.UsageFlags(vk::ImageUsageFlagBits::eTransferSrc)
						.SampleCount(vk::SampleCountFlagBits::e2)
						.InitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
						.Build()).get();
				m_images.at(idGui).emplace_back(guiImage);
			}
		}

		// auto depthPassDepthDescription = vk::AttachmentDescription()
		// 	.setFormat(vk::Format::eD32SfloatS8Uint)
		// 	.setSamples(vk::SampleCountFlagBits::e2)
		// 	.setLoadOp(vk::AttachmentLoadOp::eClear)
		// 	.setStoreOp(vk::AttachmentStoreOp::eStore)
		// 	.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		// 	.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		// 	.setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
		// 	.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		//
		// auto depthPassDepthReference = vk::AttachmentReference()
		// 	.setAttachment(0)
		// 	.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		//
		// auto depthAttachment = Graphics::RenderPass::Attachment {
		// 	.description = depthPassDepthDescription,
		// 	.reference = depthPassDepthReference,
		// 	.images = m_images.at(idDepth),
		// 	.clearValue = vk::ClearDepthStencilValue(1.0f, 0)
		// };
		//
		// auto depthSubpass = Graphics::RenderPass::Subpass {
		// 	.depthStencilAttachment = depthPassDepthReference
		// };
		//
		// m_renderPasses.emplace("depth", Graphics::RenderPass::Builder()
		// 	.OutputImageIndex(0)
		// 	.Extent({ 1920u, 1080u })
		// 	.Attachment(0, depthAttachment)
		// 	.Subpass(depthSubpass)
		// 	.ImageCount(m_frameCount)
		// 	.Build());

		auto colorPassDepthDescription = vk::AttachmentDescription()
			.setFormat(vk::Format::eD32SfloatS8Uint)
			.setSamples(vk::SampleCountFlagBits::e2)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		auto colorPassDepthReference = vk::AttachmentReference()
			.setAttachment(1)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		auto colorAttachmentDepth = Graphics::RenderPass::Attachment {
			.description = colorPassDepthDescription,
			.reference = colorPassDepthReference,
			.images = m_images.at(idDepth),
			.clearValue = vk::ClearDepthStencilValue(1.0f, 0)
		};

		auto colorPassColorDescription = vk::AttachmentDescription()
			.setFormat(vk::Format::eB8G8R8A8Unorm)
			.setSamples(vk::SampleCountFlagBits::e2)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		auto colorPassColorReference = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		auto colorAttachmentColor = Graphics::RenderPass::Attachment {
			.description = colorPassColorDescription,
			.reference = colorPassColorReference,
			.images = m_images.at(idColor),
			.clearValue = vk::ClearColorValue(std::array { 0.0f, 0.0f, 0.0f, 0.0f })
		};

		auto colorPassColorResolveDescription = vk::AttachmentDescription()
			.setFormat(vk::Format::eB8G8R8A8Unorm)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		auto colorPassColorResolveReference = vk::AttachmentReference()
			.setAttachment(2)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		auto colorAttachmentColorResolve = Graphics::RenderPass::Attachment {
			.description = colorPassColorResolveDescription,
			.reference = colorPassColorResolveReference,
			.images = m_images.at(idColorResolve),
			.clearValue = vk::ClearColorValue(std::array { 0.0f, 0.0f, 0.0f, 0.0f })
		};

		auto colorSubpass = Graphics::RenderPass::Subpass{
			.colorAttachments = {colorPassColorReference},
			.resolveAttachments = {colorPassColorResolveReference},
			.depthStencilAttachment = colorPassDepthReference
		};

		m_renderPasses.emplace("color", Graphics::RenderPass::Builder()
			.OutputImageIndex(2)
			.Attachment(0, colorAttachmentColor)
			.Attachment(1, colorAttachmentDepth)
			.Attachment(2, colorAttachmentColorResolve)
			.Extent({ 1920u, 1080u })
			.Subpass(colorSubpass)
			.ImageCount(m_frameCount)
			.Build());

		m_dynamicRenders.emplace("shadow", std::make_unique<Graphics::DynamicRender>());

		if (m_guiEnabled)
		{
			auto guiPassColorDescription = vk::AttachmentDescription()
				.setFormat(vk::Format::eB8G8R8A8Unorm)
				.setSamples(vk::SampleCountFlagBits::e2)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
				.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

			auto guiPassColorReference = vk::AttachmentReference()
				.setAttachment(0)
				.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

			auto guiPassColor = Graphics::RenderPass::Attachment {
				.description = guiPassColorDescription,
				.reference = guiPassColorReference,
				.images = m_images.at(idGui),
				.clearValue = vk::ClearColorValue(std::array { 0.0f, 0.0f, 0.0f, 1.0f })
			};

			auto guiSubpass = Graphics::RenderPass::Subpass {
				.colorAttachments = { guiPassColorReference },
			};

			m_guiRenderPass = Graphics::RenderPass::Builder()
				.OutputImageIndex(0)
				.Attachment(0, guiPassColor)
				.Extent( Math::Vector2u { 1920u, 1080u } )
				.Subpass(guiSubpass)
				.ImageCount(m_frameCount)
				.Build();
		}

		m_queues[vk::QueueFlagBits::eGraphics] = Context::Device().RequestQueue(vk::QueueFlagBits::eGraphics);

		AddNode(std::make_unique<ShadowRunNode>(*this, *m_dynamicRenders.at("shadow"), Math::Vector2u { 2048u, 2048u }, 1u));
		AddNode(std::make_unique<RenderPassRunNode>(*this, std::vector<std::string> { "color" }));

		const auto& queue = *m_queues.at(vk::QueueFlagBits::eGraphics);
		RunNode* currentNode = m_rootNode.get();
		while (currentNode != nullptr) {
			auto& commandBuffers = currentNode->commandBuffers;
			for (uint32_t i = 0; i < m_frameCount; i++) {
				commandBuffers.emplace_back(Context::Device().RequestCommandBuffer(queue));
			}
			currentNode = currentNode->nextNode.get();
		}

		{
			auto vertexShader = Shader::Manager::Get().GetShader("depth", "vertex");

			auto pipelineBuilder = std::make_unique<Graphics::Pipeline::BuilderDynamic>(vk::PipelineRenderingCreateInfo()
				.setColorAttachmentFormats({})
				.setDepthAttachmentFormat(vk::Format::eD32SfloatS8Uint)
				.setStencilAttachmentFormat(vk::Format::eD32SfloatS8Uint));
			(*pipelineBuilder)
				.AddShader(vertexShader)
				.Rasterizer(vk::PipelineRasterizationStateCreateInfo()
					.setPolygonMode(vk::PolygonMode::eFill)
					.setCullMode(vk::CullModeFlagBits::eNone)
					.setFrontFace(vk::FrontFace::eClockwise)
					.setLineWidth(1.0f))
				.InputAssemblyState(vk::PipelineInputAssemblyStateCreateInfo()
					.setTopology(vk::PrimitiveTopology::eTriangleList)
					.setPrimitiveRestartEnable(vk::False))
				.RenderFunction([](const Graphics::Pipeline& pipeline, const Core::CommandBuffer& commandBuffer) {
					pipeline.Bind(*commandBuffer);
					pipeline.BindDescriptorSet(0, *commandBuffer, ECS::SceneManager::Get().GetLoadedScene().DescriptorSet());
					ECS::SceneManager::Get().Registry().group(entt::get<ECS::Entity*, ECS::RenderTarget>).each(
						[&](const ECS::Entity* entity, const ECS::RenderTarget& renderTarget) {
							Math::Matrix4<f32> matrix = Math::Matrix4<f32>::Identity();
							while (entity) {
								auto& transform = entity->Get<ECS::Transform>();
								matrix *= transform.Matrix();
								entity = entity->Parent();
							}
							for (const auto mesh : renderTarget.Targets() | std::views::keys) {
								pipeline.PushConstants<Math::Matrix4<f32>>(*commandBuffer, vk::ShaderStageFlagBits::eVertex, 0, matrix);
								mesh->Bind(*commandBuffer);
								mesh->Draw(*commandBuffer);
							}
						}
					);
				});
			m_dynamicRenders.at("shadow")->AddPipeline(std::move(pipelineBuilder));
		}

		// TODO: Delete this:
		{
			auto* vertexShader = Shader::Manager::Get().GetShader("wireframe", "vertexMain");
			auto* fragmentShader = Shader::Manager::Get().GetShader("wireframe", "fragmentMain");

			auto pipelineBuilder = std::make_unique<Graphics::Pipeline::BuilderRenderPass>(*m_renderPasses.at("color"));
			(*pipelineBuilder)
				.AddShader(vertexShader)
				.AddShader(fragmentShader)
				.Rasterizer(vk::PipelineRasterizationStateCreateInfo()
					.setPolygonMode(vk::PolygonMode::eFill)
					.setCullMode(vk::CullModeFlagBits::eNone)
					.setFrontFace(vk::FrontFace::eClockwise)
					.setLineWidth(1.0f))
				.InputAssemblyState(vk::PipelineInputAssemblyStateCreateInfo()
					.setTopology(vk::PrimitiveTopology::eTriangleList)
					.setPrimitiveRestartEnable(vk::False))
				.RenderFunction([](const Graphics::Pipeline& pipeline, const Core::CommandBuffer& commandBuffer) {
					pipeline.Bind(*commandBuffer);
					pipeline.BindDescriptorSet(0, *commandBuffer, ECS::SceneManager::Get().GetLoadedScene().DescriptorSet());
					ECS::SceneManager::Get().Registry().group(entt::get<ECS::Entity*, ECS::RenderTarget>).each(
						[&](const ECS::Entity* entity, const ECS::RenderTarget& renderTarget) {
							Math::Matrix4<f32> matrix = Math::Matrix4<f32>::Identity();
							while (entity) {
								auto& transform = entity->Get<ECS::Transform>();
								matrix *= transform.Matrix();
								entity = entity->Parent();
							}
							for (const auto [mesh, material] : renderTarget.Targets()) {
								pipeline.BindDescriptorSet(1, *commandBuffer, material->DescriptorSet());
								pipeline.PushConstants<Math::Matrix4<f32>>(*commandBuffer, vk::ShaderStageFlagBits::eVertex, 0, matrix);
								mesh->Bind(*commandBuffer);
								mesh->Draw(*commandBuffer);
							}
						}
					);
				});

			m_pipelineBuilder = pipelineBuilder.get();
			m_renderPasses.at("color")->AddPipeline(std::move(pipelineBuilder));
		}

		// {
		// 	auto* amplificationShader = Shader::Manager::Get().GetShader("planet", "cullChunks");
		// 	auto* meshShader = Shader::Manager::Get().GetShader("planet", "renderChunkMesh");
		// 	auto* pixelShader = Shader::Manager::Get().GetShader("planet", "planetPixelShader");
		//
		// 	auto pipelineBuilder = std::make_unique<Graphics::Pipeline::Builder>(*m_renderPasses.at("color"));
		// 	(*pipelineBuilder)
		// 		.AddShader(amplificationShader)
		// 		.AddShader(meshShader)
		// 		.AddShader(pixelShader)
		// 		.Rasterizer(vk::PipelineRasterizationStateCreateInfo()
		// 			.setPolygonMode(vk::PolygonMode::eFill)
		// 			.setCullMode(vk::CullModeFlagBits::eBack)
		// 			.setFrontFace(vk::FrontFace::eClockwise)
		// 			.setLineWidth(1.0f))
		// 		.RenderFunction([](const Graphics::Pipeline& pipeline, const Core::CommandBuffer& commandBuffer) {
		// 			pipeline.Bind(*commandBuffer);
		// 			pipeline.BindDescriptorSet(0, *commandBuffer, ECS::SceneManager::Get().GetLoadedScene().PlanetDescriptorSet());
		// 			pipeline.BindDescriptorSet(1, *commandBuffer, ECS::SceneManager::Get().GetLoadedScene().PlanetMaterial().DescriptorSet());
		// 			commandBuffer->drawMeshTasksEXT(8, 8, 8);
		// 		});
		//
		// 	m_pipelineBuilder = pipelineBuilder.get();
		// 	m_renderPasses.at("color")->AddPipeline(std::move(pipelineBuilder));
		// }

		// ------------------

		if (m_guiEnabled) {
			const auto guiCreateInfo = Reef::Manager::CreateInfo {
				.queue = *m_queues.at(vk::QueueFlagBits::eGraphics),
				.renderPass = *m_guiRenderPass,
				.frameCount = m_frameCount,
				.imageFormat = vk::Format::eB8G8R8A8Unorm,
				.sampleCount = vk::SampleCountFlagBits::e2
			};

			m_guiManager = std::make_unique<Reef::Manager>(guiCreateInfo);

			for (uint32_t i = 0; i < m_frameCount; i++) {
				m_guiCommandBuffers.emplace_back(Context::Device().RequestCommandBuffer(queue));
			}

			auto& finalRenderPass = *m_renderPasses.at("color").get();
			m_viewport = Reef::MakeContainer<Reef::Viewport>(finalRenderPass);
		}
	}

	RenderGraph::~RenderGraph() {
		m_viewport.reset();
	}

	void RenderGraph::Update(const float deltaTime) const
	{
		for (const auto& renderPass : m_renderPasses | std::views::values) {
			renderPass->Update(deltaTime);
		}
		if (m_guiEnabled) {
			m_guiManager->Update(deltaTime);
		}
	}

	void RenderGraph::Execute(const Core::Frame& frame) const {
		const auto& queue = *m_queues.at(vk::QueueFlagBits::eGraphics);

		const RunNode* currentNode = m_rootNode.get();
		while (currentNode != nullptr) {
			currentNode->ExecuteNode(frame, queue);
			currentNode = currentNode->nextNode.get();
		}

		if (m_guiEnabled) {
            const auto& guiCommandBuffer = *m_guiCommandBuffers[frame.ImageIndex()];
            guiCommandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);

            guiCommandBuffer->begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
			m_guiRenderPass->Begin(guiCommandBuffer, frame.ImageIndex());
            m_guiManager->Render(guiCommandBuffer);
			m_guiRenderPass->End(guiCommandBuffer);
			guiCommandBuffer->end();

            const auto guiCommandBuffers = std::array { *guiCommandBuffer };

			constexpr auto destMask = vk::PipelineStageFlags()
				| vk::PipelineStageFlagBits::eColorAttachmentOutput;

			std::vector signalSemaphores = { frame.ReadyToPresent() };

            const auto guiSubmitInfo = vk::SubmitInfo()
                .setCommandBuffers(guiCommandBuffers)
                .setWaitSemaphores(m_lastNode->commandBuffers[frame.ImageIndex()]->SignalSemaphore())
                .setSignalSemaphores(signalSemaphores)
                .setWaitDstStageMask(destMask);

            try {
                queue->submit(guiSubmitInfo);
            } catch (const vk::OutOfDateKHRError&) {
                // Recreate framebuffers
            }
        }
	}

	void RenderGraph::Resize(const Math::Vector2<f32>& size, const bool inner) {
		if (m_guiEnabled && !inner) {
			m_guiRenderPass->Resize(m_frameCount, size);
		} else {
			for (const auto& renderPass : m_renderPasses | std::views::values) {
				renderPass->Resize(m_frameCount, size);
			}
		}
	}

	const Memory::Image& RenderGraph::OutputImage(const uint32_t frameIndex) const {
		if (m_guiEnabled) {
            return m_guiRenderPass->OutputImage(frameIndex);
        }

		std::string lastPassName = "";
		const RunNode* currentNode = m_rootNode.get();
		while (currentNode->nextNode != nullptr) {
			if (currentNode->LastPassName() != "") {
				lastPassName = currentNode->LastPassName();
			}
			currentNode = currentNode->nextNode.get();
		}
		return m_renderPasses.at(lastPassName)->OutputImage(frameIndex);
	}

	vk::Semaphore RenderGraph::RenderFinished(const uint32_t frameIndex) const {
		if (m_guiEnabled) {
			return m_guiCommandBuffers[frameIndex]->SignalSemaphore();
		}
		return m_lastNode->commandBuffers[frameIndex]->SignalSemaphore();
	}
	void RenderGraph::AddNode(std::unique_ptr<RunNode> node) {
		if (m_rootNode == nullptr) {
			m_lastNode = node.get();
			m_rootNode = std::move(node);
		} else {
			node->previousNode = m_lastNode;
			m_lastNode->nextNode = std::move(node);
			m_lastNode = m_lastNode->nextNode.get();
		}
	}

	void RenderGraph::OnGUIAttach() {
		AddDockable("Graphics Pipeline",
			new Reef::Window(ICON_FA_PAINTBRUSH "   Graphics Pipeline",
				Reef::Style{
				    .size = {300.f, 0.f},
				    .padding = {10.f, 10.f, 10.f, 10.f},
				    .spacing = 10.f,
				    .backgroundColor = {0.0f, 0.0f, 0.0f, 1.f},
					.direction = Reef::Axis::Vertical,
				},
				{
					m_pipelineTemplate->Build(
					dynamic_cast<Graphics::Pipeline::BuilderRenderPass&>(*m_pipelineBuilder)),
				}
			)
		);
	}
}
