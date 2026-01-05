//
// Created by radue on 1/4/2026.
//

#include "light.h"

#include "context.h"
#include "core/scheduler.h"
Coral::ECS::Light::Light(const Type type, const Data& data, bool castsShadows) :
	m_type(type), m_data(data), m_castsShadows(castsShadows) {
	if (!castsShadows) {
		return;
	}
	switch (type) {
	case Type::Directional: {
		for (u32 i = 0; i < Context::Scheduler().Frames().size(); i++) {
			auto shadowMap = Memory::Image::Builder()
								 .Extent(Math::Vector2u{2048u, 2048u})
								 .Format(vk::Format::eD32SfloatS8Uint)
								 .UsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment)
								 .UsageFlags(vk::ImageUsageFlagBits::eSampled)
								 .MipLevels(1)
								 .LayerCount(1)
								 .InitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
								 .SampleCount(vk::SampleCountFlagBits::e1)
								 .Build();
			m_shadowMaps.emplace_back(std::move(shadowMap));
		}
		break;
	}
	case Type::Point: {
		for (u32 i = 0; i < Context::Scheduler().Frames().size(); i++) {
			auto shadowMap = Memory::Image::Builder()
								 .Extent(Math::Vector2u{1024u, 1024u})
								 .Format(vk::Format::eD32Sfloat)
								 .UsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment)
								 .UsageFlags(vk::ImageUsageFlagBits::eSampled)
								 .MipLevels(1)
								 .LayerCount(6)
								 .InitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
								 .SampleCount(vk::SampleCountFlagBits::e1)
								 .Build();
			m_shadowMaps.emplace_back(std::move(shadowMap));
		}
		break;
	}
	case Type::Spot: {
		for (u32 i = 0; i < Context::Scheduler().Frames().size(); i++) {
			auto shadowMap = Memory::Image::Builder()
								 .Extent(Math::Vector2u{2048u, 2048u})
								 .Format(vk::Format::eD32SfloatS8Uint)
								 .UsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment)
								 .UsageFlags(vk::ImageUsageFlagBits::eSampled)
								 .MipLevels(1)
								 .LayerCount(1)
								 .InitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
								 .SampleCount(vk::SampleCountFlagBits::e1)
								 .Build();
			m_shadowMaps.emplace_back(std::move(shadowMap));
		}
		break;
	}
	}

	auto cameraCI = Camera::CreateInfo {
		.projectionData = Camera::ProjectionData(
			Camera::Orthographic {
				.left = -10.0f,
				.right = 10.0f,
				.top = 10.0f,
				.bottom = -10.0f,
				.near = 0.1f,
				.far = 100.0f,
			}
		),
		.size = Math::Vector2u{2048u, 2048u},
	};
	m_shadowCamera = std::make_unique<Camera>(cameraCI);
}
const Coral::Memory::Image& Coral::ECS::Light::ShadowMap(const u32 frameIndex) const {
	return *m_shadowMaps.at(frameIndex);
}
