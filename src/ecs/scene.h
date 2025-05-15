//
// Created by radue on 10/24/2024.
//
#pragma once

#include <memory>

#include "gui/layer.h"

#include <entt/entt.hpp>

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

        static Scene& Get();

        [[nodiscard]] Entity& Root() const { return *m_root; }
        [[nodiscard]] entt::registry& Registry() { return m_registry; }

        Camera& MainCamera();

    private:
        Reef::EntityInspector* m_inspectorTemplate;

        std::unique_ptr<Entity> m_root;
        entt::entity m_selectedObject = entt::null;
        entt::registry m_registry;

        inline static Scene* m_currentScene = nullptr;
    };
}
