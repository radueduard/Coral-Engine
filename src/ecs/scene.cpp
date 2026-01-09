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

#include "core/input.h"
#include "core/scheduler.h"
#include "memory/gpuStructs.h"

namespace Coral::ECS {
    Scene::Scene() {
    	Context::m_scene = this;

    	m_inspectorTemplate = new Reef::EntityInspector();
    	m_root = std::make_unique<ECS::Entity>("Root");
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
					new Reef::TreeView<ECS::Entity, entt::entity>(
						*m_root,
						[this](ECS::Entity& object) {
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
    	auto firstCamera = std::make_unique<ECS::Entity>("Camera");
    	auto firstCameraCreateInfo = Camera::CreateInfo {
    		.projectionData = Camera::ProjectionData(Camera::Type::Perspective),
			.size = { 800u, 600u }
    	};
    	auto& camera = firstCamera->Add<Camera>(firstCameraCreateInfo);
    	firstCamera->Get<Transform>().position.z = 30.0f;
    	firstCamera->Get<Camera>().Primary() = true;

    	m_root->Add<Camera>(firstCameraCreateInfo);
    	m_root->AddChild(std::move(firstCamera));

    	m_setLayout = Memory::Descriptor::SetLayout::Builder()
			.AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
			.Build();

    	m_set = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), *m_setLayout)
			.WriteBuffer(0, camera.Buffer().DescriptorInfo())
			.Build();

    	m_planetSetLayout = Memory::Descriptor::SetLayout::Builder()
    		.AddBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eMeshEXT)
    		.AddBinding(1, vk::DescriptorType::eUniformBuffer,
    			vk::ShaderStageFlagBits::eMeshEXT
    			| vk::ShaderStageFlagBits::eTaskEXT
    		)
    		.Build();

		constexpr u32 resolution = 1024;
    	// m_planetNoise = std::make_unique<Utils::CircleNoise<3>>(Math::Vector3u(resolution), 0.5f);
		m_planetNoise = std::make_unique<Utils::PerlinNoise3D>(Math::Vector3u(resolution), 9);

    	m_planetImageView = Memory::ImageView::Builder(m_planetNoise->Image())
    		.ViewType(vk::ImageViewType::e3D)
    		.Build();

    	m_planetSampler = Memory::Sampler::Builder()
			.Build();

		m_planetSet = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), *m_planetSetLayout)
    		.WriteImage(0, vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(**m_planetImageView)
				.setSampler(**m_planetSampler))
			.WriteBuffer(1, camera.Buffer().DescriptorInfo())
			.Build();

    	const auto albedoUUID = Asset::Manager::Get().LoadTextureFromFile("assets/textures/stylized_grass/stylized-grass1_albedo.png");
    	const auto normalUUID = Asset::Manager::Get().LoadTextureFromFile("assets/textures/stylized_grass/stylized-grass1_normal-dx.png");
    	const auto roughnessUUID = Asset::Manager::Get().LoadTextureFromFile("assets/textures/stylized_grass/stylized-grass1_roughness.png");
		const auto metallicUUID = Asset::Manager::Get().LoadTextureFromFile("assets/textures/stylized_grass/stylized-grass1_metallic.png");
    	const auto aoUUID = Asset::Manager::Get().LoadTextureFromFile("assets/textures/stylized_grass/stylized-grass1_ao.png");

    	m_planetMaterial = Graphics::Material::Builder()
			.Name("Planet Material")
			.AddTexture(PBR::Usage::Albedo, Asset::Manager::Get().GetTexture(albedoUUID))
    		.AddTexture(PBR::Usage::Normal, Asset::Manager::Get().GetTexture(normalUUID))
			.AddTexture(PBR::Usage::Roughness, Asset::Manager::Get().GetTexture(roughnessUUID))
			.AddTexture(PBR::Usage::Metallic, Asset::Manager::Get().GetTexture(metallicUUID))
			.AddTexture(PBR::Usage::AmbientOcclusion, Asset::Manager::Get().GetTexture(aoUUID))
			.RoughnessFactor(1.0f)
			.MetallicFactor(0.0f)
			.DoubleSided(false)
			.Build();

    	m_shadowCastingLightCount = 0;

    	m_lightCameraBuffer = Memory::Buffer::Builder()
			.InstanceSize(sizeof(GPU::Camera))
			.InstanceCount(16)
			.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
			.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
			.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
			.Build();

    	m_shadowMapArray.resize(Context::Scheduler().Frames().size());
		for (u32 i = 0; i < Context::Scheduler().Frames().size(); i++) {
			m_shadowMapArray[i] = Memory::Image::Builder()
	    		.Type(vk::ImageType::e2D)
				.Extent(Math::Vector2u { 2048u, 2048u })
				.Format(vk::Format::eD32Sfloat)
				.UsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment)
				.UsageFlags(vk::ImageUsageFlagBits::eSampled)
				.LayerCount(16)
				.InitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
				.Build();
		}

    	m_shadowMapViews.resize(Context::Scheduler().Frames().size());
		for (u32 i = 0; i < Context::Scheduler().Frames().size(); i++) {
			m_shadowMapViews[i] = Memory::ImageView::Builder(*m_shadowMapArray[i])
				.ViewType(vk::ImageViewType::e2DArray)
				.LayerCount(16)
				.Build();
		}

    	m_shadowMapSampler = Memory::Sampler::Builder()
    		.Build();

    	m_shadowDescriptorSetLayout = Memory::Descriptor::SetLayout::Builder()
			.AddBinding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
			.AddBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
			.Build();

    	m_shadowDescriptorSets.resize(Context::Scheduler().Frames().size());
    	for (u32 i = 0; i < Context::Scheduler().Frames().size(); i++) {
    		m_shadowDescriptorSets[i] = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), *m_shadowDescriptorSetLayout)
				.WriteBuffer(0, m_lightCameraBuffer->DescriptorInfo())
				.WriteImage(1, vk::DescriptorImageInfo()
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setImageView(**m_shadowMapViews[i])
					.setSampler(**m_shadowMapSampler))
				.Build();
    	}
    }

	void Scene::Update(const float deltaTime) {
		auto& mainCamera = ViewCamera();

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
				displacement.y -= 1.0f;
			}
			if (Input::IsKeyHeld(Key::E)) {
				displacement.y += 1.0f;
			}
			if (displacement.Length() > 0.0f) {
				mainCamera.Move(displacement * deltaTime * 3.0f);
			}

			const Math::Vector2<f32> mouseDelta = Input::GetMousePositionDelta() * 5.f;
			mainCamera.Rotate(mouseDelta.x, -mouseDelta.y);
		}

    	for (const auto& child : *m_root) {
			child.Update();
		}
    }

	ECS::Entity& Scene::Entity(const entt::entity entityId) const {
	    return *SceneManager::Get().Registry().get<ECS::Entity*>(entityId);
    }

	Camera& Scene::PrimaryCamera() {
    	auto& registry = SceneManager::Get().Registry();
		for (const auto cameras = registry.view<Camera>(); const auto camera : cameras) {
			if (registry.get<Camera>(camera).Primary()) {
				return registry.get<Camera>(camera);
			}
		}
		throw std::runtime_error("No primary camera found");
	}

	Camera& Scene::ViewCamera() {
		auto& registry = SceneManager::Get().Registry();
		for (const auto cameras = registry.view<Camera>(); const auto camera : cameras) {
			if (registry.get<Camera>(camera).Primary()) {
				return registry.get<Camera>(camera);
			}
		}
		throw std::runtime_error("No view camera found");
	}

	Entity* Scene::SelectedEntity() const {
	    if (m_selectedObject == entt::null) {
		    return nullptr;
	    }
    	return SceneManager::Get().Registry().get<ECS::Entity*>(m_selectedObject);
    }
	std::pair<std::vector<std::unique_ptr<Memory::ImageView>>, u32> Scene::GetShadowMap() {
    	std::vector<std::unique_ptr<Memory::ImageView>> shadowMaps;
    	u32 index = m_shadowCastingLightCount++;
    	for (u32 i = 0; i < Context::Scheduler().Frames().size(); i++) {
    		shadowMaps.emplace_back(Memory::ImageView::Builder(*m_shadowMapArray[i])
				.ViewType(vk::ImageViewType::e2D)
				.BaseArrayLayer(index)
				.LayerCount(1)
				.Build());
		}
	    return { std::move(shadowMaps), index };
    }
}
