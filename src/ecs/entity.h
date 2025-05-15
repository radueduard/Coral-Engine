//
// Created by radue on 10/23/2024.
//

#pragma once

#include <entt/entity/entity.hpp>

#include "components/transform.h"
#include "utils/narryTree.h"

#include "scene.h"


namespace Coral::ECS {
    class Entity final : public NarryTree<Entity, entt::entity> {
    public:
        explicit Entity(String name = "Empty") {
            auto& registry = Scene::Get().Registry();
            m_id = registry.create();
            registry.emplace<Entity*>(m_id, this);
            registry.emplace<Transform>(m_id);
            m_name = std::move(name);
        }
        ~Entity() override = default;

        Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;

        // [[nodiscard]] entt::entity Id() const override { return m_id; }
        [[nodiscard]] const String &Name() const { return m_name; }
        String& Name() { return m_name; }

        template<typename T, typename... Args>
        T& Add(Args&&... args) const {
            auto& registry = Scene::Get().Registry();
            return registry.emplace<T>(m_id, std::forward<Args>(args)...);
        }

        template<typename T>
        [[nodiscard]]
        bool Has() const {
            const auto& registry = Scene::Get().Registry();
            return registry.all_of<T>(m_id);
        }

        template<typename T>
        T& Get() const {
            auto& registry = Scene::Get().Registry();
            if (registry.all_of<T>(m_id)) {
                return registry.get<T>(m_id);
            }
            throw std::runtime_error("Component not found");
        }

        template<typename T>
        void Remove() const {
            auto& registry = Scene::Get().Registry();
            if (registry.all_of<T>(m_id)) {
                registry.remove<T>(m_id);
            }
        }

        bool operator==(const Entity& other) const override {
            return m_id == other.m_id;
        }
        bool operator!=(const Entity& other) const override {
            return m_id != other.m_id;
        }

    private:
        String m_name;
    };
}

inline std::string to_string(const Coral::ECS::Entity& object) {
    return object.Name();
}