//
// Created by radue on 1/4/2026.
//

#include "light.h"

#include "context.h"
#include "core/scheduler.h"
#include "memory/gpuStructs.h"

Coral::ECS::Light::Light(const Type type, const Data& data, bool castsShadows) :
	m_type(type), m_data(data), m_castsShadows(castsShadows) {
	if (!castsShadows) {
		return;
	}
	switch (type) {
	case Type::Directional:
	case Type::Spot: {
		auto [shadowMaps, index] = Context::Scene().GetShadowMap();
		m_shadowMaps = std::move(shadowMaps);
		m_index = index;
		break;
	}
	default:
		throw std::runtime_error("Only Directional and Spot lights can cast shadows for now!");
	}
}

void Coral::ECS::Light::Setup() {
	auto& transform = Entity().Get<Transform>();
	transform.rotation = Math::Degrees<f32, 3>(Math::Quaternion<>::ToEulerAngles(Math::LookAt(m_data.directional.direction, Math::Vector3<f32>(0.f, 1.f, 0.f))));

	const auto& camera = Entity().Add<Camera>(Camera::CreateInfo {
		.projectionData = Camera::ProjectionData(Camera::Orthographic {
			.left = -40.f,
			.right = 40.f,
			.top = 40.f,
			.bottom = -40.f,
			.near = -100.f,
			.far = 100.f,
		}),
		.size = { 2048u, 2048u },
	});

	m_shadowDescriptorSetLayout = Memory::Descriptor::SetLayout::Builder()
		.AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
		.Build();

	m_shadowDescriptorSet = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), *m_shadowDescriptorSetLayout)
		.WriteBuffer(0, camera.Buffer().DescriptorInfo())
		.Build();

	switch (m_type) {
	case Type::Directional:
	case Type::Spot: {
		auto& buffer = Context::Scene().LightCameraBuffer();
		buffer.Map<GPU::Camera>();
		buffer.WriteAt(m_index, GPU::Camera {
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
	if (transform.Changed()) {
		auto& camera = Entity().Get<Camera>();
		switch (m_type) {
		case Type::Directional:
		case Type::Spot: {
			auto& buffer = Context::Scene().LightCameraBuffer();
			buffer.Map<GPU::Camera>();
			buffer.WriteAt(m_index, GPU::Camera {
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
}

const Coral::Memory::ImageView& Coral::ECS::Light::ShadowMap(const u32 frameIndex) const {
	return *m_shadowMaps.at(frameIndex);
}
