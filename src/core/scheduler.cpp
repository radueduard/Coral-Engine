//
// Created by radue on 10/14/2024.
//

#include "scheduler.h"

#include "context.h"
#include "ecs/entity.h"
#include "graphics/renderPass.h"
#include "gui/elements/popup.h"
#include "memory/descriptor/pool.h"
#include "project/renderGraph.h"


namespace Coral::Core {
	Frame::Frame(const uint32_t imageIndex, const Core::Queue& queue)
	 : m_imageIndex(imageIndex), m_queue(queue)
	{
		m_imageAvailable = Context::Device()->createSemaphore({});
		m_readyToPresent = Context::Device()->createSemaphore({});
		// m_inFlightFence = Context::Device()->createFence(vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled));
		// m_queue = Context::Device().RequestQueue(vk::QueueFlagBits::eGraphics);
		// m_finalImageTransferCommandBuffer = Context::Device().RequestCommandBuffer(*m_queue);

		m_finalImageTransferCommandBuffer = Context::Device().RequestCommandBuffer(m_queue);

	}

	Frame::~Frame() {
		Context::Device()->destroySemaphore(m_imageAvailable);
		Context::Device()->destroySemaphore(m_readyToPresent);
		// Context::Device()->destroyFence(m_inFlightFence);
	}

    Scheduler::Scheduler(const CreateInfo& createInfo)
        : m_imageCount(createInfo.imageCount)
    {
		static bool firstTime = true;
		if (!firstTime) {
			throw std::runtime_error("Scheduler already created!");
		}
		firstTime = false;
        Context::m_scheduler = this;

		m_queue = Context::Device().RequestQueue(vk::QueueFlagBits::eGraphics);
        CreateFrames();

        const auto swapChainCreateInfo = Graphics::SwapChain::CreateInfo {
            .minImageCount = createInfo.minImageCount,
            .imageCount = m_imageCount,
            .sampleCount = createInfo.multiSampling,
        };
        m_swapChain = std::make_unique<Graphics::SwapChain>(swapChainCreateInfo);

        CreateDescriptorPool();

        const auto renderGraphCreateInfo = Project::RenderGraph::CreateInfo {
            .frameCount = m_imageCount,
            .guiEnabled = true,
        };

        m_renderGraph = Reef::MakeContainer<Project::RenderGraph>(renderGraphCreateInfo);
    }

    Scheduler::~Scheduler() {
        Context::Device()->waitIdle();
    }

    void Scheduler::CreateDescriptorPool() {
        m_descriptorPool = Memory::Descriptor::Pool::Builder()
            .AddPoolSize(vk::DescriptorType::eUniformBuffer, 100)
            .AddPoolSize(vk::DescriptorType::eStorageBuffer, 100)
            .AddPoolSize(vk::DescriptorType::eCombinedImageSampler, 100)
            .AddPoolSize(vk::DescriptorType::eStorageImage, 100)
            .PoolFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .MaxSets(100)
            .Build();
    }

    void Scheduler::CreateFrames() {
        m_frames.reserve(m_imageCount);
        for (uint32_t i = 0; i < m_imageCount; i++) {
            m_frames.emplace_back(std::make_unique<Frame>(i, *m_queue));
        }
    }

    void Scheduler::Update(const float deltaTime) {
        m_renderGraph->Update(deltaTime);
        // m_renderGraph->Resize(m_window.Extent());
    }

    void Scheduler::Draw() {
        const auto& frame = CurrentFrame();

        const auto& fence = frame.InFlightFence();
        if (const auto result = Context::Device()->waitForFences(1, &fence, vk::True, UINT64_MAX); result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to wait fence: " + vk::to_string(result));
        }

        if (const auto result = Context::Device()->resetFences(1, &fence); result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to reset fence: " + vk::to_string(result));
        }

        if (const auto result = m_swapChain->Acquire(frame);
            result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            m_swapChain->Resize(Window::Get().Extent());
            m_renderGraph->Resize(Window::Get().Extent());
            return;
        }

    	m_renderGraph->Execute(frame);

        frame.FinalImageTransferCommandBuffer().Run([&](const CommandBuffer& commandBuffer) {
            const Memory::Image& outputImage = m_renderGraph->OutputImage(frame.ImageIndex());
            const Memory::Image& swapChainImage = *m_swapChain->SwapChainImages()[frame.ImageIndex()];

            vk::ImageMemoryBarrier outputImageBarrier = vk::ImageMemoryBarrier()
                .setImage(*outputImage)
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
                .setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

            commandBuffer->pipelineBarrier(
                vk::PipelineStageFlagBits::eAllGraphics,
                vk::PipelineStageFlagBits::eAllCommands,
                vk::DependencyFlags(),
                nullptr,
                nullptr,
                { outputImageBarrier }
            );


            if (outputImage.SampleCount() != vk::SampleCountFlagBits::e1) {
                const auto imageResolve = vk::ImageResolve()
                    .setSrcSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
                    .setSrcOffset(vk::Offset3D(0, 0, 0))
                    .setDstSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
                    .setDstOffset(vk::Offset3D(0, 0, 0))
                    .setExtent(vk::Extent3D(outputImage.Extent()));

                commandBuffer->resolveImage(
                    *outputImage,
                    vk::ImageLayout::eTransferSrcOptimal,
                    *swapChainImage,
                    vk::ImageLayout::eTransferDstOptimal,
                    imageResolve
                );
            } else {
                vk::ImageMemoryBarrier swapChainImageMemoryBarrier = vk::ImageMemoryBarrier()
                    .setImage(*swapChainImage)
                    .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                    .setOldLayout(vk::ImageLayout::ePresentSrcKHR)
                    .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

                commandBuffer->pipelineBarrier(
                    vk::PipelineStageFlagBits::eAllGraphics,
                    vk::PipelineStageFlagBits::eAllCommands,
                    vk::DependencyFlags(),
                    nullptr,
                    nullptr,
                    { swapChainImageMemoryBarrier }
                );

                commandBuffer->copyImage(
                    *outputImage,
                    vk::ImageLayout::eTransferSrcOptimal,
                    *swapChainImage,
                    vk::ImageLayout::eTransferDstOptimal,
                    { vk::ImageCopy()
                        .setSrcSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
                        .setDstSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
                        .setExtent(vk::Extent3D(outputImage.Extent())) }
                );

                swapChainImageMemoryBarrier
                    .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                    .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setNewLayout(vk::ImageLayout::ePresentSrcKHR);

                commandBuffer->pipelineBarrier(
                    vk::PipelineStageFlagBits::eAllGraphics,
                    vk::PipelineStageFlagBits::eAllCommands,
                    vk::DependencyFlags(),
                    nullptr,
                    nullptr,
                    { swapChainImageMemoryBarrier }
                );
            }

            outputImageBarrier
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);

            commandBuffer->pipelineBarrier(
                vk::PipelineStageFlagBits::eAllCommands,
                vk::PipelineStageFlagBits::eAllCommands,
                vk::DependencyFlags(),
                nullptr,
                nullptr,
                { outputImageBarrier }
            );
        }, frame.ReadyToPresent());

        if (const auto result = m_swapChain->Present(frame);
            result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            m_swapChain->Resize(Window::Get().Extent());
            m_renderGraph->Resize(Window::Get().Extent());
        }

		AdvanceFrame();
    }
}
