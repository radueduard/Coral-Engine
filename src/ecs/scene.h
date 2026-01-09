//
// Created by radue on 10/24/2024.
//
#pragma once

#include <memory>

#include "gui/layer.h"

#include <entt/entt.hpp>

#include "graphics/objects/material.h"
#include "utils/noise.h"

namespace Coral::Memory {
	class Sampler;
	class ImageView;
	class Image;
	class Buffer;
}
namespace Coral::Memory::Descriptor {
	class SetLayout;
	class Set;
}
namespace Coral::Reef {
    class EntityInspector;
}

namespace Coral::ECS {
    class Camera;
    class Entity;

    class Scene final : public Reef::Layer {
    public:
        explicit Scene();
		~Scene() override = default;

        void OnGUIAttach() override;
		void Setup();

		void Update(float deltaTime);

    	[[nodiscard]] ECS::Entity& Entity(entt::entity entityId) const;
        [[nodiscard]] ECS::Entity& Root() const { return *m_root; }

    	[[nodiscard]] Memory::Descriptor::Set& DescriptorSet() const { return *m_set; }

		static Camera& PrimaryCamera();
		static Camera& ViewCamera();

		ECS::Entity* SelectedEntity() const;

    	[[nodiscard]] Memory::Descriptor::Set& PlanetDescriptorSet() const { return *m_planetSet; }
    	[[nodiscard]] Graphics::Material& PlanetMaterial() const { return *m_planetMaterial; }

    	std::pair<std::vector<std::unique_ptr<Memory::ImageView>>, u32> GetShadowMap();

    	Memory::Buffer& LightCameraBuffer() const { return *m_lightCameraBuffer; }

    	Memory::Descriptor::Set& ShadowDescriptorSet(const u32 index) const { return *m_shadowDescriptorSets[index]; }

    	Memory::Image& ShadowMap(const u32 index) const { return *m_shadowMapArray[index]; }

    private:

        Reef::EntityInspector* m_inspectorTemplate;

        std::unique_ptr<ECS::Entity> m_root;
        entt::entity m_selectedObject = entt::null;

    	std::unique_ptr<Memory::Descriptor::SetLayout> m_setLayout;
    	std::unique_ptr<Memory::Descriptor::Set> m_set;

    	std::unique_ptr<Utils::Noise> m_planetNoise;
    	std::unique_ptr<Memory::ImageView> m_planetImageView;
    	std::unique_ptr<Memory::Sampler> m_planetSampler;

    	std::unique_ptr<Memory::Descriptor::SetLayout> m_planetSetLayout;
    	std::unique_ptr<Memory::Descriptor::Set> m_planetSet;
    	std::unique_ptr<Graphics::Material> m_planetMaterial;

    	u32 m_shadowCastingLightCount;
    	std::unique_ptr<Memory::Buffer> m_lightCameraBuffer;

    	std::vector<std::unique_ptr<Memory::Image>> m_shadowMapArray;
		std::vector<std::unique_ptr<Memory::ImageView>> m_shadowMapViews;

    	std::unique_ptr<Memory::Descriptor::SetLayout> m_shadowDescriptorSetLayout;
    	std::vector<std::unique_ptr<Memory::Descriptor::Set>> m_shadowDescriptorSets;
    };
}
