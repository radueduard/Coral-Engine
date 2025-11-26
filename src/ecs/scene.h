//
// Created by radue on 10/24/2024.
//
#pragma once

#include <memory>

#include "gui/layer.h"

#include <entt/entt.hpp>

namespace Coral::Memory {
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

        Camera& MainCamera();

    private:

        Reef::EntityInspector* m_inspectorTemplate;

        std::unique_ptr<Entity> m_root = nullptr;
        entt::entity m_selectedObject = entt::null;

    	std::unique_ptr<Memory::Descriptor::SetLayout> m_setLayout;
    	std::unique_ptr<Memory::Descriptor::Set> m_set;
    	std::unique_ptr<Memory::Buffer> m_cameraBuffer;
    };
}
