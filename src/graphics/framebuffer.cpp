//
// Created by radue on 3/6/2025.
//

#include "framebuffer.h"

Graphics::Framebuffer::Framebuffer(const RenderPass &renderPass, const uint32_t index): m_renderPass(renderPass) {
	std::vector<vk::ImageView> attachments;
	for (const auto& attachment : renderPass.Attachments()) {
		const auto& imageView = *m_imageViews.emplace_back(Memory::ImageView::Builder(*attachment.images[index])
			.ViewType(vk::ImageViewType::e2D)
			.BaseMipLevel(0)
			.LevelCount(1)
			.Build());
		attachments.emplace_back(*imageView);
	}

	const auto createInfo = vk::FramebufferCreateInfo()
		.setRenderPass(*renderPass)
		.setAttachments(attachments)
		.setWidth(renderPass.Extent().x)
		.setHeight(renderPass.Extent().y)
		.setLayers(1);

	m_handle = Core::GlobalDevice()->createFramebuffer(createInfo);
}

Graphics::Framebuffer::~Framebuffer() {
	Core::GlobalDevice()->destroyFramebuffer(m_handle);
}
