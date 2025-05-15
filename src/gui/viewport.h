//
// Created by radue on 3/6/2025.
//

#pragma once
#include <spirv_cross/spirv_common.hpp>

#include "imgui_impl_vulkan.h"
#include "layer.h"
#include "graphics/framebuffer.h"
#include "graphics/renderPass.h"
#include "memory/sampler.h"

#include "reef.h"

namespace Coral::Reef {
	class Viewport final : public Layer {
	public:
		explicit Viewport(Graphics::RenderPass& renderPass);

		void OnGUIAttach() override;
		void OnGUIDetach() override;
		void OnGUIUpdate() override;

	private:
		Graphics::RenderPass& m_renderPass;

		std::unique_ptr<Memory::Sampler> m_sampler;
		std::vector<vk::DescriptorSet> m_viewportTextures;
		Image* m_image = nullptr;
    };
}
