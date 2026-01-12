//
// Created by radue on 1/4/2026.
//

#include "light.h"

#include "context.h"
#include "core/scheduler.h"
#include "memory/gpuStructs.h"

#include "math/constants.h"

Coral::ECS::Light::Light(const LightType type, bool castsShadows) :
	type(type), m_castsShadows(castsShadows) {

	m_index = Context::Scene().AllocateNewLight(type);

	if (!castsShadows) {
		return;
	}
	switch (type) {
	case LightType::Directional:
	case LightType::Spot: {
		auto [shadowMaps, index] = Context::Scene().GetShadowMap();
		m_shadowMaps = std::move(shadowMaps);
		m_shadowMapIndex = index;
		break;
	}
	default:
		throw std::runtime_error("Only Directional and Spot lights can cast shadows for now!");
	}
}

template<>
auto Coral::ECS::Light::GPU<Coral::ECS::LightType::Directional>() const {
	const auto& transform = Entity().Get<Transform>();
	return GPU::Light::Directional {
		.direction = Math::Direction(Math::Radians<f32, 3>(transform.rotation)),
		.color = {color.r, color.g, color.b, color.a },
		.intensity = intensity,
	};
}

template<>
auto Coral::ECS::Light::GPU<Coral::ECS::LightType::Spot>() const {
	const auto& transform = Entity().Get<Transform>();
	return GPU::Light::Spot {
		.position = transform.position,
		.range = range,
		.direction = Math::Direction(Math::Radians<f32, 3>(transform.rotation)),
		.intensity = intensity,
		.color = { color.r, color.g, color.b, color.a },
		.attenuation = attenuation,
		.innerAngle = innerAngle,
		.outerAngle = outerAngle,
	};
}

template<>
auto Coral::ECS::Light::GPU<Coral::ECS::LightType::Point>() const {
	const auto& transform = Entity().Get<Transform>();
	return GPU::Light::Point {
		.position = transform.position,
		.intensity = intensity,
		.color = { color.r, color.g, color.b, color.a },
		.attenuation = attenuation,
		.range = range
	};
}

void Coral::ECS::Light::Setup() {
	if (!m_castsShadows) {
		return;
	}

	Camera::ProjectionData projectionData;
	switch (type) {
	case LightType::Directional:
		projectionData = Camera::ProjectionData(Camera::Orthographic {
			.left = -40.f,
			.right = 40.f,
			.top = 40.f,
			.bottom = -40.f,
			.near = -100.f,
			.far = 100.f,
		});
		break;
	case LightType::Spot:
		projectionData = Camera::ProjectionData(Camera::Perspective {
			.fov = 90.f,
			.near = 1.f,
			.far = 100.f,
		});
		break;
	default:
		throw std::runtime_error("Only Directional and Spot lights can cast shadows for now!");
	}

	const auto& camera = Entity().Add<Camera>(Camera::CreateInfo {
		.projectionData = projectionData,
		.size = { 2048u, 2048u },
	});

	m_shadowDescriptorSetLayout = Memory::Descriptor::SetLayout::Builder()
		.AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
		.Build();

	m_shadowDescriptorSet = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), *m_shadowDescriptorSetLayout)
		.WriteBuffer(0, camera.Buffer().DescriptorInfo())
		.Build();

	switch (type) {
	case LightType::Directional:
	case LightType::Spot: {
		auto& buffer = Context::Scene().LightCameraBuffer();
		buffer.Map<GPU::Camera>();
		buffer.WriteAt(m_shadowMapIndex, GPU::Camera {
			.view = camera.View(),
			.projection = camera.Projection(),
		});
		buffer.Unmap();
		break;
	}
	default:
		break;
	}
}

void Coral::ECS::Light::Update()
{
	auto& transform = Entity().Get<Transform>();
	if (transform.Changed() && m_castsShadows) {
		auto& camera = Entity().Get<Camera>();
		switch (type) {
		case LightType::Directional:
		case LightType::Spot: {
			auto& buffer = Context::Scene().LightCameraBuffer();
			buffer.Map<GPU::Camera>();
			buffer.WriteAt(m_shadowMapIndex, GPU::Camera {
				.view = camera.View(),
				.projection = camera.Projection(),
			});
			buffer.Unmap();
			break;
		}
		default:
			break;
		}
	}

	if (m_changed || transform.Changed()) {
		m_changed = false;
		auto& buffer = Context::Scene().LightBuffer(type);
		switch (type) {
		case LightType::Point:
			buffer.WriteAt(m_index, GPU<LightType::Point>());
			break;
		case LightType::Directional:
			buffer.WriteAt(m_index, GPU<LightType::Directional>());
			break;
		case LightType::Spot:
			buffer.WriteAt(m_index, GPU<LightType::Spot>());
			break;
		default:
			throw std::runtime_error("No light type found");
		}
	}
}

const Coral::Memory::ImageView& Coral::ECS::Light::ShadowMap(const u32 frameIndex) const {
	return *m_shadowMaps.at(frameIndex);
}
