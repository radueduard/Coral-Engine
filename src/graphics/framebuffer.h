//
// Created by radue on 3/6/2025.
//

#pragma once
#include "renderPass.h"
#include "utils/globalWrapper.h"

namespace Graphics {
	class Framebuffer final : public EngineWrapper<vk::Framebuffer> {
	public:
		explicit Framebuffer(const RenderPass& renderPass, uint32_t index);
		~Framebuffer() override;

		[[nodiscard]] const Memory::ImageView& ImageView(const uint32_t index) const { return *m_imageViews[index]; }

	private:
		const RenderPass& m_renderPass;
		std::vector<std::unique_ptr<Memory::ImageView>> m_imageViews;
	};
}
