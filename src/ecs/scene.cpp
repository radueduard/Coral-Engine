//
// Created by radue on 10/24/2024.
//

#include "scene.h"
#include <queue>
#include "IconsFontAwesome6.h"
#include "context.h"

#include "assets/manager.h"

#include "components/camera.h"
#include "components/light.h"
#include "components/script.h"

#include "gui/reef.h"

#include "ecs/entity.h"
#include "gui/templates/inspector.h"

#include "core/input.h"
#include "core/scheduler.h"
#include "core/time.h"

#include "memory/gpuStructs.h"

#include "utils/noise.h"

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

    	firstCamera->Add<Script>([] (const ECS::Entity& self) {
    		auto& camera = self.Get<Camera>();
    		// camera.SetUpDirection({0.0f, 0.0f, 1.0f});
    		// camera.SetForwardDirection({1.0f, 0.0f, 0.0f});
    	}, [] (const ECS::Entity& self) {
    		auto& camera = self.Get<Camera>();
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
					camera.Move(displacement * Time::FrameTime<float>() * 3.0f);
				}

    			// const auto& transform = self.Get<Transform>();
    			// camera.SetUpDirection(transform.position.Normalized());

				const Math::Vector2<f32> mouseDelta = Input::GetMousePositionDelta() * 5.f;
				camera.Rotate(mouseDelta.x, -mouseDelta.y);
			}
    	});

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

    	auto albedoUUID = Context::AssetManager().LoadTextureFromFile("assets/textures/stylized_grass/stylized-grass1_albedo.png");
    	auto normalUUID = Context::AssetManager().LoadTextureFromFile("assets/textures/stylized_grass/stylized-grass1_normal-dx.png");
    	auto roughnessUUID = Context::AssetManager().LoadTextureFromFile("assets/textures/stylized_grass/stylized-grass1_roughness.png");
		auto metallicUUID = Context::AssetManager().LoadTextureFromFile("assets/textures/stylized_grass/stylized-grass1_metallic.png");
    	auto aoUUID = Context::AssetManager().LoadTextureFromFile("assets/textures/stylized_grass/stylized-grass1_ao.png");

    	m_planetMaterial = Graphics::Material::Builder()
			.Name("Planet Material")
			.AddTexture(PBR::Usage::Albedo, Context::AssetManager().GetTexture(albedoUUID))
    		.AddTexture(PBR::Usage::Normal, Context::AssetManager().GetTexture(normalUUID))
			.AddTexture(PBR::Usage::Roughness, Context::AssetManager().GetTexture(roughnessUUID))
			.AddTexture(PBR::Usage::Metallic, Context::AssetManager().GetTexture(metallicUUID))
			.AddTexture(PBR::Usage::AmbientOcclusion, Context::AssetManager().GetTexture(aoUUID))
			.RoughnessFactor(1.0f)
			.MetallicFactor(0.0f)
			.DoubleSided(false)
			.Build();

    	albedoUUID = Context::AssetManager().LoadTextureFromFile("assets/textures/snow_packed/snow-packed12-Base_Color.png");
    	normalUUID = Context::AssetManager().LoadTextureFromFile("assets/textures/snow_packed/snow-packed12-Normal-dx.png");
    	roughnessUUID = Context::AssetManager().LoadTextureFromFile("assets/textures/snow_packed/snow-packed12-Roughness.png");
    	metallicUUID = Context::AssetManager().LoadTextureFromFile("assets/textures/snow_packed/snow-packed12-Metallic.png");
    	aoUUID = Context::AssetManager().LoadTextureFromFile("assets/textures/snow_packed/snow-packed12-ao.png");

    	m_waterMaterial = Graphics::Material::Builder()
			.Name("Water Material")
			.AddTexture(PBR::Usage::Albedo, Context::AssetManager().GetTexture(albedoUUID))
			.AddTexture(PBR::Usage::Normal, Context::AssetManager().GetTexture(normalUUID))
			.AddTexture(PBR::Usage::Roughness, Context::AssetManager().GetTexture(roughnessUUID))
			.AddTexture(PBR::Usage::Metallic, Context::AssetManager().GetTexture(metallicUUID))
			.AddTexture(PBR::Usage::AmbientOcclusion, Context::AssetManager().GetTexture(aoUUID))
    		.BaseColorFactor({0.0f, 0.3f, 0.8f, 1.0f})
			.RoughnessFactor(.5f)
			.MetallicFactor(1.0f)
			.DoubleSided(false)
			.Build();

    	m_shadowCastingLightCount = 0;

    	m_lightCameraBuffer = Memory::Buffer::Builder()
			.InstanceSize(sizeof(GPU::Camera))
			.InstanceCount(16)
    		.DeviceAlignment(4)
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

    	m_pointLightCount = 0;
    	m_pointLightBuffer = Memory::Buffer::Builder()
    		.InstanceSize(sizeof(GPU::Light::Point))
    		.InstanceCount(16)
			.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
			.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
			.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
			.Build();

    	m_directionalLightCount = 0;
    	m_directionalLightBuffer = Memory::Buffer::Builder()
    		.InstanceSize(sizeof(GPU::Light::Directional))
    		.InstanceCount(16)
    		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
    		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
    		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
    		.Build();

    	m_spotLightCount = 0;
    	m_spotLightBuffer = Memory::Buffer::Builder()
			.InstanceSize(sizeof(GPU::Light::Spot))
			.InstanceCount(16)
			.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
			.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
			.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
			.Build();

    	m_lightsDescriptorSetLayout = Memory::Descriptor::SetLayout::Builder()
			.AddBinding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
			.AddBinding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
			.AddBinding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
			.Build();

    	m_lightsDescriptorSet = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), *m_lightsDescriptorSetLayout)
			.WriteBuffer(0, m_pointLightBuffer->DescriptorInfo())
			.WriteBuffer(1, m_directionalLightBuffer->DescriptorInfo())
			.WriteBuffer(2, m_spotLightBuffer->DescriptorInfo())
			.Build();
    }

	void Scene::Update() {
    	m_pointLightBuffer->Map<GPU::Light::Point>();
    	m_directionalLightBuffer->Map<GPU::Light::Directional>();
    	m_spotLightBuffer->Map<GPU::Light::Spot>();
    	for (const auto& child : *m_root) {
			child.Update();
		}
    	m_pointLightBuffer->Unmap();
		m_directionalLightBuffer->Unmap();
		m_spotLightBuffer->Unmap();
    }

	ECS::Entity& Scene::Entity(const entt::entity entityId) const {
	    return *Context::SceneManager().Registry().get<ECS::Entity*>(entityId);
    }

	Camera& Scene::PrimaryCamera() {
    	auto& registry = Context::SceneManager().Registry();
		for (const auto cameras = registry.view<Camera>(); const auto camera : cameras) {
			if (registry.get<Camera>(camera).Primary()) {
				return registry.get<Camera>(camera);
			}
		}
		throw std::runtime_error("No primary camera found");
	}

	Camera& Scene::ViewCamera() {
		auto& registry = Context::SceneManager().Registry();
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
    	return Context::SceneManager().Registry().get<ECS::Entity*>(m_selectedObject);
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

	u32 Scene::AllocateNewLight(const ECS::LightType& type) {
    	switch (type) {
    	case ECS::LightType::Point:
    		return m_pointLightCount++;
    	case ECS::LightType::Spot:
    		return m_spotLightCount++;
    	case ECS::LightType::Directional:
    		return m_directionalLightCount++;
    	default:
    		throw std::runtime_error("No new light type found");
    	}
    }

	Math::Vector4u Scene::LightCounts() const {
    	return { m_pointLightCount, m_directionalLightCount, m_spotLightCount, m_shadowCastingLightCount };
	}

	Memory::Buffer & Scene::LightBuffer(const ECS::LightType &type) const {
		switch (type) {
			case LightType::Point:
				return *m_pointLightBuffer;
			case LightType::Spot:
				return *m_spotLightBuffer;
			case LightType::Directional:
				return *m_directionalLightBuffer;
			default:
				throw std::runtime_error("Unknown type");
		}
	}
}
