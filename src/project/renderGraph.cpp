//
// Created by radue on 3/6/2025.
//

#include "renderGraph.h"

#include <queue>
#include <boost/uuid/nil_generator.hpp>

#include "core/scheduler.h"
#include "graphics/pipeline.h"
#include "gui/container.h"
#include "gui/viewport.h"

namespace Project {
	RenderGraph::RenderGraph(const CreateInfo& createInfo)
		: m_guiEnabled(createInfo.guiEnabled), m_frameCount(createInfo.frameCount) {
		m_generator = boost::uuids::random_generator_mt19937();

		vk::Extent3D extent = { createInfo.window.Extent().width, createInfo.window.Extent().height, 1 };

		auto idGui = boost::uuids::nil_uuid();
		if (m_guiEnabled) {
            idGui = m_generator();
            m_images.emplace(idGui, std::vector<Memory::Image*>());
        }

		auto idDepth = m_generator();
		auto idColor = m_generator();
		m_images.emplace(idDepth, std::vector<Memory::Image*>());
		m_images.emplace(idColor, std::vector<Memory::Image*>());

		for (uint32_t i = 0; i < m_frameCount; i++) {
			Memory::Image* depthImage = m_imageStorage.emplace_back(
				Memory::Image::Builder()
					.Format(vk::Format::eD32Sfloat)
					.Extent(extent)
					.UsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment)
					.SampleCount(vk::SampleCountFlagBits::e1)
					.InitialLayout(vk::ImageLayout::eUndefined)
					.Build()).get();
			m_images.at(idDepth).emplace_back(depthImage);

			Memory::Image* colorImage = m_imageStorage.emplace_back(
				Memory::Image::Builder()
					.Format(vk::Format::eR8G8B8A8Srgb)
					.Extent(extent)
					.UsageFlags(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc)
					.SampleCount(vk::SampleCountFlagBits::e1)
					.InitialLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.Build()).get();
			m_images.at(idColor).emplace_back(colorImage);

			if (m_guiEnabled) {
				Memory::Image* guiImage = m_imageStorage.emplace_back(
					Memory::Image::Builder()
						.Format(vk::Format::eB8G8R8A8Unorm)
						.Extent(extent)
						.UsageFlags(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc)
						.SampleCount(vk::SampleCountFlagBits::e2)
						.InitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
						.Build()).get();
				m_images.at(idGui).emplace_back(guiImage);
			}
		}

		auto depthPassDepthDescription = vk::AttachmentDescription()
			.setFormat(vk::Format::eD32Sfloat)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		auto depthPassDepthReference = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		auto depthAttachment = Graphics::RenderPass::Attachment {
			.description = depthPassDepthDescription,
			.reference = depthPassDepthReference,
			.images = m_images.at(idDepth),
			.clearValue = vk::ClearDepthStencilValue(1.0f, 0)
		};

		auto depthSubpass = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setPDepthStencilAttachment(&depthPassDepthReference);

		m_renderPasses.emplace("depth", Graphics::RenderPass::Builder()
			.OutputImageIndex(0)
			.Extent({ 1920, 1080 })
			.Attachment(0, depthAttachment)
			.Subpass(depthSubpass)
			.ImageCount(m_frameCount)
			.Build());

		auto colorPassDepthDescription = vk::AttachmentDescription()
			.setFormat(vk::Format::eD32Sfloat)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eLoad)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
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
			.setFormat(vk::Format::eR8G8B8A8Srgb)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		auto colorPassColorReference = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		auto colorAttachmentColor = Graphics::RenderPass::Attachment {
			.description = colorPassColorDescription,
			.reference = colorPassColorReference,
			.images = m_images.at(idColor),
			.clearValue = vk::ClearColorValue(std::array { 0.0f, 0.0f, 0.0f, 1.0f })
		};

		auto colorSubpass = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(colorPassColorReference)
			.setPDepthStencilAttachment(&colorPassDepthReference);

		m_renderPasses.emplace("color", Graphics::RenderPass::Builder()
			.OutputImageIndex(0)
			.Attachment(0, colorAttachmentColor)
			.Attachment(1, colorAttachmentDepth)
			.Extent({ 1920, 1080 })
			.Subpass(colorSubpass)
			.ImageCount(m_frameCount)
			.Build());

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

			auto guiSubpass = vk::SubpassDescription()
				.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
				.setColorAttachments(guiPassColorReference);

			m_guiRenderPass = Graphics::RenderPass::Builder()
				.OutputImageIndex(0)
				.Attachment(0, guiPassColor)
				.Extent({ 1920, 1080 })
				.Subpass(guiSubpass)
				.ImageCount(m_frameCount)
				.Build();
		}

		m_queues[vk::QueueFlagBits::eGraphics] = Core::GlobalDevice().RequestQueue(vk::QueueFlagBits::eGraphics);

		m_runNodes.emplace_back(std::make_unique<RunNode>(std::vector<std::string> { "depth" }));
		m_runNodes.emplace_back(std::make_unique<RunNode>(std::vector<std::string> { "color" }));

		const auto& queue = *m_queues.at(vk::QueueFlagBits::eGraphics);
		for (auto& node : m_runNodes) {
			auto&[passes, commandBuffers] = *node;
            for (uint32_t i = 0; i < m_frameCount; i++) {
                commandBuffers.emplace_back(Core::GlobalDevice().RequestCommandBuffer(queue.Family().Index()));
            }
        }

		// TODO: Delete this:

		const auto vertexShader = Core::Shader("shaders/wireframe/wireframe.vert");
		const auto fragmentShader = Core::Shader("shaders/wireframe/wireframe.frag");

		std::unique_ptr<Graphics::Pipeline> pipeline = Graphics::Pipeline::Builder(*m_renderPasses.at("color").get())
			.AddShader(&vertexShader)
			.AddShader(&fragmentShader)
			.Build();

		m_renderPasses.at("color")->AddPipeline(std::move(pipeline));

		// ------------------

		if (m_guiEnabled) {
			const auto guiCreateInfo = GUI::Manager::CreateInfo {
				.window =  createInfo.window,
				.runtime = createInfo.runtime,
				.queue = *m_queues.at(vk::QueueFlagBits::eGraphics),
				.renderPass = *m_guiRenderPass,
				.frameCount = m_frameCount,
				.imageFormat = vk::Format::eB8G8R8A8Unorm,
				.sampleCount = vk::SampleCountFlagBits::e2
			};

			m_guiManager = std::make_unique<GUI::Manager>(guiCreateInfo);
			GUI::g_manager = m_guiManager.get();

			for (uint32_t i = 0; i < m_frameCount; i++) {
				m_guiCommandBuffers.emplace_back(Core::GlobalDevice().RequestCommandBuffer(queue.Family().Index()));
			}

			auto& finalRenderPass = *m_renderPasses.at("color").get();
			m_viewport = GUI::MakeContainer<GUI::Viewport>(finalRenderPass);
		}
	}

	void RenderGraph::Update(const float deltaTime) const
	{
		if (m_guiEnabled) {
			m_guiManager->Update(deltaTime);
		}
	}

	void RenderGraph::Execute(const Core::Frame& frame) {
		const auto& queue = *m_queues.at(vk::QueueFlagBits::eGraphics);
		for (int i = 0; i < m_runNodes.size(); i++) {
			const auto& commandBuffer = *m_runNodes[i]->commandBuffers[frame.ImageIndex()];
			const auto& commands = m_runNodes[i]->passes;

            commandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
			commandBuffer->begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

			for (const auto& renderPass : commands) {
				m_renderPasses.at(renderPass)->Begin(commandBuffer, frame.ImageIndex());
				m_renderPasses.at(renderPass)->End(commandBuffer);
			}
			commandBuffer->end();

			const auto commandBuffers = std::array { *commandBuffer };

			std::vector<vk::Semaphore> waitSemaphores;
			if (i > 0) {
				const auto& previousCommandBuffer = *m_runNodes[i - 1]->commandBuffers[frame.ImageIndex()];
				waitSemaphores.emplace_back(previousCommandBuffer.SignalSemaphore());
			} else {
				waitSemaphores.emplace_back(frame.ImageAvailable());
			}

			constexpr auto destMask = vk::PipelineStageFlags()
				| vk::PipelineStageFlagBits::eColorAttachmentOutput;

			const auto submitInfo = vk::SubmitInfo()
				.setCommandBuffers(commandBuffers)
				.setWaitSemaphores(waitSemaphores)
				.setSignalSemaphores(commandBuffer.SignalSemaphore())
				.setWaitDstStageMask(destMask);

			try {
                queue->submit(submitInfo);
            } catch (const vk::OutOfDateKHRError&) {
                // Recreate framebuffers
            }
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

            const auto guiSubmitInfo = vk::SubmitInfo()
                .setCommandBuffers(guiCommandBuffers)
                .setWaitSemaphores(m_runNodes.back()->commandBuffers[frame.ImageIndex()]->SignalSemaphore())
                .setSignalSemaphores(m_guiCommandBuffers[frame.ImageIndex()]->SignalSemaphore())
                .setWaitDstStageMask(destMask);

            try {
                queue->submit(guiSubmitInfo);
            } catch (const vk::OutOfDateKHRError&) {
                // Recreate framebuffers
            }
        }
	}

	void RenderGraph::Resize(const vk::Extent2D& extent)
	{
		if (m_guiEnabled) {
			m_guiRenderPass->Resize(m_frameCount, extent);
		} else
		{
			for (const auto& renderPass : m_renderPasses | std::views::values) {
				renderPass->Resize(m_frameCount, extent);
			}
		}
	}

	const Memory::Image& RenderGraph::OutputImage(const uint32_t frameIndex) const {
		if (m_guiEnabled) {
            return m_guiRenderPass->OutputImage(frameIndex);
        }
		const auto lastPassName = m_runNodes.back()->passes.back();
		return m_renderPasses.at(lastPassName)->OutputImage(frameIndex);
	}

	vk::Semaphore RenderGraph::RenderFinished(const uint32_t frameIndex) const
	{
		if (m_guiEnabled) {
			return m_guiCommandBuffers[frameIndex]->SignalSemaphore();
        }
		return m_runNodes.back()->commandBuffers[frameIndex]->SignalSemaphore();
	}
}
