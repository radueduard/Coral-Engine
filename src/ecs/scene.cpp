//
// Created by radue on 10/24/2024.
//

#include "scene.h"
#include <queue>
#include "IconsFontAwesome6.h"
#include "context.h"

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
			new Reef::Window(ICON_FA_LIST "   Scene View",
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
								new Reef::Window(
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
			.size = { 800u, 600u }
    	};
    	firstCamera->Add<Camera>(firstCameraCreateInfo);
    	firstCamera->Get<Transform>().position.z = 3.0f;
    	firstCamera->Get<Camera>().Primary() = true;

    	m_root->Add<Camera>(firstCameraCreateInfo);
    	m_root->AddChild(std::move(firstCamera));

    	m_setLayout = Memory::Descriptor::SetLayout::Builder()
			.AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eTessellationEvaluation)
			.Build();

		m_cameraBuffer = Memory::Buffer::Builder()
    		.InstanceCount(1)
    		.InstanceSize(sizeof(GPU::Camera))
    		.UsageFlags(vk::BufferUsageFlagBits::eUniformBuffer)
    		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
    		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
    		.Build();

    	m_set = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), *m_setLayout)
			.WriteBuffer(0, m_cameraBuffer->DescriptorInfo())
			.Build();
    }

	void Scene::Update(const float deltaTime) {
		auto& mainCamera = MainCamera();

		if (Input::IsMouseButtonHeld(MouseButton::MouseButtonRight)) {
			Math::Vector3f displacement { 0.0f, 0.0f, 0.0f };
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
