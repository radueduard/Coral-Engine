//
// Created by radue on 6/17/2025.
//

#pragma once

#include "color/color.h"
#include "component.h"

#include "ecs/entity.h"

#include "math/vector.h"

namespace Coral::ECS {
	class Camera;
}
namespace Coral::Memory {
	class ImageView;
	class Buffer;
	namespace Descriptor {
		class SetLayout;
		class Set;
	}
	class Image;
}

namespace Coral::ECS {
	enum class LightType : u8 {
		Point,
		Directional,
		Spot
	};

	struct Light final : Component {
		LightType type;
		Color color = Colors::white;
		f32 intensity = 1.f;
		Math::Vector3<f32> attenuation = { 1.f, 0.09f, 0.032f };
		f32 range = 10.f;
		f32 innerAngle = 30.f; // in degrees
		f32 outerAngle = 45.f; // in degrees

		explicit Light(LightType type = LightType::Directional, bool castsShadows = false);

		template<LightType T>
		auto GPU() const;

		Light(const Light&) = delete;
		Light& operator=(const Light&) = delete;

		~Light() override = default;

		void Setup() override;
		void Update() override;

		bool m_changed = true;
		bool m_castsShadows = false;

		u32 m_index = 0;

		u32 m_shadowMapIndex = 0;
		std::vector<std::unique_ptr<Memory::ImageView>> m_shadowMaps;

		[[nodiscard]] Memory::Descriptor::Set& ShadowDescriptorSet() const {
			return *m_shadowDescriptorSet;
		}

		[[nodiscard]] bool CastsShadows() const {
			return m_castsShadows;
		}

		const Memory::ImageView& ShadowMap(u32 frameIndex) const;
	private:
		std::unique_ptr<Memory::Buffer> m_lightCameraBuffer;
		std::unique_ptr<Memory::Descriptor::SetLayout> m_shadowDescriptorSetLayout;
		std::unique_ptr<Memory::Descriptor::Set> m_shadowDescriptorSet;
	};
}
