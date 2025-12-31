//
// Created by radue on 10/24/2024.
//
#pragma once

#include <memory>

#include "gui/layer.h"

#include <entt/entt.hpp>

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

        [[nodiscard]] Entity& Root() const { return *m_root; }

    	[[nodiscard]] Memory::Descriptor::Set& DescriptorSet() const { return *m_set; }

    	const Memory::Buffer& CameraBuffer() const { return *m_cameraBuffer; }

        Camera& MainCamera();
    	Entity* SelectedEntity() const;

    	[[nodiscard]] Memory::Descriptor::Set& PlanetDescriptorSet() const { return *m_planetSet; }

    private:

        Reef::EntityInspector* m_inspectorTemplate;

        std::unique_ptr<Entity> m_root;
        entt::entity m_selectedObject = entt::null;

    	std::unique_ptr<Memory::Descriptor::SetLayout> m_setLayout;
    	std::unique_ptr<Memory::Descriptor::Set> m_set;
    	std::unique_ptr<Memory::Buffer> m_cameraBuffer;

    	std::unique_ptr<Utils::Noise> m_planetNoise;
    	std::unique_ptr<Memory::ImageView> m_planetImageView;
    	std::unique_ptr<Memory::Sampler> m_planetSampler;

    	std::unique_ptr<Memory::Descriptor::SetLayout> m_planetSetLayout;
    	std::unique_ptr<Memory::Descriptor::Set> m_planetSet;
    };
}
