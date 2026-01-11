//
// Created by radue on 10/23/2024.
//

#pragma once

#include <entt/entity/entity.hpp>

#include "context.h"

#include "scene.h"
#include "gui/container.h"
#include "sceneManager.h"

#include "components/transform.h"
#include "utils/narryTree.h"

namespace Coral::ECS {
	template<typename T> requires std::is_base_of_v<Component, T>
	constexpr u16 TypeMask();

    class Entity final : public Tree<Entity, entt::entity> {
    public:
        explicit Entity(String name = "Empty");

		~Entity() override;

		Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;

        // [[nodiscard]] entt::entity Id() const override { return m_id; }
        [[nodiscard]] const String &Name() const { return m_name; }
        String& Name() { return m_name; }

    	std::unique_ptr<Entity> Clone() const;

		template<typename T, typename... Args> requires std::is_base_of_v<Component, T>
        T& Add(Args&&... args) {
            auto& registry = Context::SceneManager().Registry();
            T& component = registry.emplace<T>(m_id, std::forward<Args>(args)...);
        	component.SetEntity(this->m_id);
        	m_componentMask |= TypeMask<T>();
        	component.Setup();
			return component;
        }

    	void Update() const;

		template <typename T>
        [[nodiscard]]
        bool Has() const {
            const auto& registry = Context::SceneManager().Registry();
            return registry.all_of<T>(m_id);
        }

        template<typename T>
        T& Get() const {
            auto& registry = Context::SceneManager().Registry();
            if (registry.all_of<T>(m_id)) {
                return registry.get<T>(m_id);
            }
            throw std::runtime_error("Component not found");
        }

        template<typename T>
        void Remove() {
            auto& registry = Context::SceneManager().Registry();
            if (registry.all_of<T>(m_id)) {
                registry.remove<T>(m_id);
            	m_componentMask &= ~T::componentTypeMask;
            }
        }

    	void AddEmpty();
		void AddCamera();
		void AddLight(const LightType& type);
		void AddCube();
		void AddSphere();

	private:
        String m_name;
    	u16 m_componentMask;
    };
}

inline std::string to_string(const Coral::ECS::Entity& entity) {
	return entity.Name();
}
