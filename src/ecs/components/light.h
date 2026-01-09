//
// Created by radue on 6/17/2025.
//

#pragma once

#include "color/color.h"
#include "component.h"
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
	struct Light final : Component {
		struct Point {
			Color color = Colors::white;
			Math::Vector3<f32> attenuation = { 1.f, 0.09f, 0.032f };
			f32 range = 10.f;
		};

		struct Directional {
			Color color = Colors::white;
			f32 intensity = 1.f;
			Math::Vector3<f32> direction = { 0.f, -1.f, 0.f };
		};

		struct Spot {
			Color color = Colors::white;
			f32 intensity = 1.f;
			f32 range = 10.f;
			f32 innerAngle = 30.f; // in degrees
			f32 outerAngle = 45.f; // in degrees
			Math::Vector3<f32> direction = { 0.f, -1.f, 0.f };
		};

		enum class Type : uint8_t {
			Point,
			Directional,
			Spot
		};

		union Data {
			Directional directional;
			Point point;
			Spot spot;

			Data() { directional = Directional(); }
			explicit Data(const Directional& dir) : directional(dir) {}
			explicit Data(const Point& pt) : point(pt) {}
			explicit Data(const Spot& spt) : spot(spt) {}

			~Data() {}
		};

		explicit Light(Type type = Type::Directional, const Data& data = Data {}, bool castsShadows = false);

		Light(const Light&) = delete;
		Light& operator=(const Light&) = delete;

		~Light() override = default;

		void Setup() override;
		void Update() override;

		bool m_changed = true;
		Type m_type;
		Data m_data;
		bool m_castsShadows = false;

		u32 m_index = 0;
		std::vector<std::unique_ptr<Memory::ImageView>> m_shadowMaps;

		Memory::Descriptor::Set& ShadowDescriptorSet() const {
			return *m_shadowDescriptorSet;
		}

		bool CastsShadows() const {
			return m_castsShadows;
		}

		const Memory::ImageView& ShadowMap(u32 frameIndex) const;
	private:
		std::unique_ptr<Memory::Buffer> m_lightCameraBuffer;
		std::unique_ptr<Memory::Descriptor::SetLayout> m_shadowDescriptorSetLayout;
		std::unique_ptr<Memory::Descriptor::Set> m_shadowDescriptorSet;
	};
}
