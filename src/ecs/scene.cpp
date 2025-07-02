//
// Created by radue on 10/24/2024.
//

#include "scene.h"
#include <queue>
#include "IconsFontAwesome6.h"

#include "components/camera.h"
#include "gui/reef.h"

#include "ecs/entity.h"
#include "gui/templates/inspector.h"

#include "components/light.h"
#include "core/input.h"
#include "core/scheduler.h"
#include "memory/gpuStructs.h"
#include "gui/elements/popup.h"

namespace Coral::ECS {
    Scene::Scene() {
    	m_inspectorTemplate = new Reef::EntityInspector();
    	m_root = std::make_unique<Entity>("Root");
    }

    void Scene::OnGUIAttach() {
		AddDockable("Scene View",
			new Reef::Dockable(ICON_FA_LIST "   Scene View",
				Reef::Style{
				   .size = {300.f, 0.f},
				   .padding = {10.f, 10.f, 10.f, 10.f},
				   .spacing = 10.f,
				   .backgroundColor = {0.0f, 0.0f, 0.0f, 1.f},
				},
				{
					new Reef::TreeView<Entity, entt::entity>(
						*m_root,
						[this](Entity& object) {
							m_selectedObject = object.Id();
							AddDockable(
								"Object Inspector",
								new Reef::Dockable(
									ICON_FA_INFO "   Object Inspector",
									Reef::Style{
										.size = {300.f, Reef::Grow},
										.padding = {10.f, 10.f, 10.f, 10.f},
										.spacing = 10.f,
										.backgroundColor = {0.0f, 0.0f, 0.0f, 1.f},
									},
									{
										new Reef::Element({}, {
											m_inspectorTemplate->Build(object),
										})
									}
								)
							);
						},
						Reef::Style{
							.size = {Reef::Grow, Reef::Grow},
							.padding = {10.f, 10.f, 10.f, 10.f},
							.cornerRadius = 10.f,
							.backgroundColor = {0.1f, 0.1f, 0.1f, 1.f},
						}),
				}));


		// m_guiBuilder["Object Inspector"] = [this] () -> GUI::Element* {
		//     if (m_selectedObject == entt::null) {
		//         return new GUI::Dockable(
		//             ICON_FA_INFO "   Object Inspector",
		//             new GUI::Text("No object selected", GUI::Text::Style{ { 0.8f, 0.8f, 0.8f, 1.f }, 20.f,
		//             GUI::FontType::Black }), { 10.f, 10.f }
		//         );
		//     }
		//     return m_inspectorTemplate.Build(*ECS::World::Get().Registry().get<ECS::Entity*>(m_selectedObject));
		// };
	}

	void Scene::Setup() {
    	auto firstCamera = std::make_unique<Entity>("Camera");
    	auto firstCameraCreateInfo = Camera::CreateInfo {
    		.projectionData = Camera::ProjectionData(Camera::Type::Perspective),
			.size = { 800, 600 }
    	};
    	firstCamera->Add<Camera>(firstCameraCreateInfo);
    	firstCamera->Get<Transform>().position.z = 3.0f;
    	firstCamera->Get<Camera>().Primary() = true;

    	m_root->Add<Camera>(firstCameraCreateInfo);
    	m_root->AddChild(std::move(firstCamera));

    	m_setLayout = Memory::Descriptor::SetLayout::Builder()
			.AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
    		.AddBinding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
			.AddBinding(2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
			.AddBinding(3, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
			.AddBinding(4, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
			.Build();

		m_cameraBuffer = Memory::Buffer::Builder()
    		.InstanceCount(1)
    		.InstanceSize(sizeof(GPU::Camera))
    		.UsageFlags(vk::BufferUsageFlagBits::eUniformBuffer)
    		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
    		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
    		.Build();

    	m_lightCountsBuffer = Memory::Buffer::Builder()
			.InstanceCount(1)
			.InstanceSize(sizeof(Math::Vector3<u32>))
			.UsageFlags(vk::BufferUsageFlagBits::eUniformBuffer)
			.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
			.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
			.Build();

    	m_pointLightBuffer = Memory::Buffer::Builder()
    		.InstanceCount(32)
    		.InstanceSize(sizeof(GPU::Light::Point))
    		.UsageFlags(vk::BufferUsageFlagBits::eUniformBuffer)
    		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
    		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
    		.Build();

    	m_directionalLightBuffer = Memory::Buffer::Builder()
    		.InstanceCount(32)
			.InstanceSize(sizeof(GPU::Light::Directional))
			.UsageFlags(vk::BufferUsageFlagBits::eUniformBuffer)
			.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
			.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
			.Build();

    	m_spotLightBuffer = Memory::Buffer::Builder()
			.InstanceCount(32)
    		.InstanceSize(sizeof(GPU::Light::Spot))
			.UsageFlags(vk::BufferUsageFlagBits::eUniformBuffer)
    		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
			.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
			.Build();

    	m_set = Memory::Descriptor::Set::Builder(Core::GlobalScheduler().DescriptorPool(), *m_setLayout)
			.WriteBuffer(0, m_cameraBuffer->DescriptorInfo())
			.WriteBuffer(1 , m_lightCountsBuffer->DescriptorInfo())
			.WriteBuffer(2, m_pointLightBuffer->DescriptorInfo())
			.WriteBuffer(3, m_directionalLightBuffer->DescriptorInfo())
			.WriteBuffer(4, m_spotLightBuffer->DescriptorInfo())
			.Build();
    }

	void Scene::Update(const float deltaTime) {
		auto& mainCamera = MainCamera();
    	auto& registry = SceneManager::Get().Registry();


		if (Input::IsMouseButtonHeld(MouseButton::MouseButtonRight)) {
			Math::Vector3 displacement { 0.0f, 0.0f, 0.0f };
			if (Input::IsKeyHeld(Key::W)) {
				displacement.z += 1.0f;
			}
			if (Input::IsKeyHeld(Key::S)) {
				displacement.z -= 1.0f;
			}
			if (Input::IsKeyHeld(Key::A)) {
				displacement.x += 1.0f;
			}
			if (Input::IsKeyHeld(Key::D)) {
				displacement.x -= 1.0f;
			}
			if (Input::IsKeyHeld(Key::Q)) {
				displacement.y += 1.0f;
			}
			if (Input::IsKeyHeld(Key::E)) {
				displacement.y -= 1.0f;
			}
			if (displacement.Length() > 0.0f) {
				mainCamera.Move(displacement * deltaTime * 3.0f);
			}

			const Math::Vector2<f32> mouseDelta = Input::GetMousePositionDelta() * 5.f;
			mainCamera.Rotate(mouseDelta.x, mouseDelta.y);
		}
		if (mainCamera.Changed() || mainCamera.Moved()) {
			mainCamera.RecalculateView();
			mainCamera.RecalculateProjection();

			m_cameraBuffer->Map<GPU::Camera>();
			m_cameraBuffer->WriteAt(0, GPU::Camera {
				.view = mainCamera.View(),
				.projection = mainCamera.Projection(),
				.inverseView = mainCamera.InverseView(),
				.inverseProjection = mainCamera.InverseProjection(),
			});
			m_cameraBuffer->Flush();
			m_cameraBuffer->Unmap();
		}

    	u32 pointLightCount = 0;
    	u32 directionalLightCount = 0;
    	u32 spotLightCount = 0;

    	bool changed = false;
		for (const auto& entity : registry.view<Light>()) {
			const auto& light = registry.get<Light>(entity);
			const auto& transform = registry.get<Transform>(entity);
			if (light.m_changed || transform.Changed()) {
				changed = true;
				registry.get<Light>(entity).m_changed = false;
			}
			if (light.m_type == Light::Type::Point) {
				pointLightCount++;
			} else if (light.m_type == Light::Type::Directional) {
				directionalLightCount++;
			} else if (light.m_type == Light::Type::Spot) {
				spotLightCount++;
			}
		}

    	if (changed) {
    		auto pointLights = m_pointLightBuffer->Map<GPU::Light::Point>();
    		auto directionalLights = m_directionalLightBuffer->Map<GPU::Light::Directional>();
    		auto spotLights = m_spotLightBuffer->Map<GPU::Light::Spot>();

    		u32 pointIndex = 0, directionalIndex = 0, spotIndex = 0;
    		for (const auto& entity : registry.view<Light>()) {
    			const auto& transform = registry.get<Transform>(entity);
				const auto& light = registry.get<Light>(entity);
				if (light.m_type == Light::Type::Point) {
					pointLights[pointIndex++] = GPU::Light::Point {
						.position = transform.position,
						.color = light.m_data.point.color,
						.attenuation = light.m_data.point.attenuation,
						.radius = light.m_data.point.range,
					};
				} else if (light.m_type == Light::Type::Directional) {
					directionalLights[directionalIndex++] = GPU::Light::Directional {
						.direction = Math::Direction(Math::Radians(transform.rotation)),
						.color = light.m_data.directional.color,
						.intensity = light.m_data.directional.intensity,
					};
				} else if (light.m_type == Light::Type::Spot) {
					spotLights[spotIndex++] = GPU::Light::Spot {
						.position = transform.position,
						.direction = Math::Direction(Math::Radians(transform.rotation)),
						.color = light.m_data.spot.color,
						.innerAngle = Math::Radians(light.m_data.spot.innerAngle),
						.outerAngle = Math::Radians(light.m_data.spot.outerAngle),
						.intensity = light.m_data.spot.intensity,
						.range = light.m_data.spot.range,
					};
				}
    		}
    		m_pointLightBuffer->Flush();
    		m_directionalLightBuffer->Flush();
    		m_spotLightBuffer->Flush();
    		m_pointLightBuffer->Unmap();
    		m_directionalLightBuffer->Unmap();
    		m_spotLightBuffer->Unmap();
    	}

    	m_lightCountsBuffer->Map<Math::Vector3<u32>>();
    	auto currentValues = m_lightCountsBuffer->ReadAt<Math::Vector3<u32>>(0);
    	m_lightCountsBuffer->WriteAt(0, Math::Vector3 { pointLightCount, directionalLightCount, spotLightCount });
    	m_lightCountsBuffer->Flush();
    	m_lightCountsBuffer->Unmap();
    }

    Camera& Scene::MainCamera() {
    	auto& registry = SceneManager::Get().Registry();
		for (const auto cameras = registry.view<Camera>(); const auto camera : cameras) {
			if (registry.get<Camera>(camera).Primary()) {
				return registry.get<Camera>(camera);
			}
		}
		throw std::runtime_error("No primary camera found");
	}
}
