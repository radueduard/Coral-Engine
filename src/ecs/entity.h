//
// Created by radue on 10/23/2024.
//

#pragma once

#include <entt/entity/entity.hpp>

#include "components/transform.h"
#include "utils/narryTree.h"

#include "assets/importer.h"
#include "assets/manager.h"
#include "components/camera.h"
#include "components/light.h"
#include "components/renderTarget.h"
#include "scene.h"
#include "sceneManager.h"


namespace Coral::ECS {
    class Entity final : public NarryTree<Entity, entt::entity> {
    public:
        explicit Entity(String name = "Empty") {
            auto& registry = SceneManager::Get().Registry();
            m_id = registry.create();
            registry.emplace<Entity*>(m_id, this);
            registry.emplace<Transform>(m_id);
            m_name = std::move(name);
        }
        ~Entity() override {
            auto& registry = SceneManager::Get().Registry();
        		if (registry.valid(m_id)) {
				registry.remove<Entity*>(m_id);
				registry.remove<Transform>(m_id);
				registry.destroy(m_id);
			}
        }

        Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;

        // [[nodiscard]] entt::entity Id() const override { return m_id; }
        [[nodiscard]] const String &Name() const { return m_name; }
        String& Name() { return m_name; }

    	std::unique_ptr<Entity> Clone() const {
        	auto& registry = SceneManager::Get().Registry();
			const auto& transform = registry.get<Transform>(m_id);

			auto newEntity = std::make_unique<Entity>(m_name);
			newEntity->m_id = registry.create();
			registry.emplace<Entity*>(newEntity->m_id, newEntity.get());
			registry.emplace<Transform>(newEntity->m_id, transform.position, transform.rotation, transform.scale);

        	for (const auto& child : Children()) {
				auto clonedChild = child->Clone();
				newEntity->AddChild(std::move(clonedChild));
			}
			return newEntity;
        }

        template<typename T, typename... Args> requires std::is_base_of_v<Component, T>
        T& Add(Args&&... args) const {
            auto& registry = SceneManager::Get().Registry();
            T& component = registry.emplace<T>(m_id, std::forward<Args>(args)...);
        	component.SetEntity(this->m_id);
			return component;
        }

        template<typename T>
        [[nodiscard]]
        bool Has() const {
            const auto& registry = SceneManager::Get().Registry();
            return registry.all_of<T>(m_id);
        }

        template<typename T>
        T& Get() const {
            auto& registry = SceneManager::Get().Registry();
            if (registry.all_of<T>(m_id)) {
                return registry.get<T>(m_id);
            }
            throw std::runtime_error("Component not found");
        }

        template<typename T>
        void Remove() const {
            auto& registry = SceneManager::Get().Registry();
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

    	void AddEmpty() {
        	static int counter = 0;
			auto child = std::make_unique<Entity>("Empty" + std::to_string(counter++));
			AddChild(std::move(child));
        }

    	void AddCamera() {
        	static int counter = 0;
        	auto child = std::make_unique<Entity>("Camera" + std::to_string(counter++));
			child->Add<Camera>(Camera::CreateInfo {});
        	AddChild(std::move(child));
        }

    	void AddLight() {
        	static int counter = 0;
			auto child = std::make_unique<Entity>("Light" + std::to_string(counter++));
			child->Add<Light>();
			AddChild(std::move(child));
		}

    	void AddCube() {
	        static int counter = 0;
        	auto child = std::make_unique<Entity>("Cube" + std::to_string(counter++));
        	auto& renderTarget = child->Add<RenderTarget>();
        	renderTarget.Add(Asset::Manager::Get().GetMesh(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000001")),
							 Asset::Manager::Get().GetMaterial(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000000")));
        	AddChild(std::move(child));
        }

    	void AddSphere() {
        	static int counter = 0;
        	auto child = std::make_unique<Entity>("Sphere" + std::to_string(counter++));
        	auto& renderTarget = child->Add<RenderTarget>();
        	renderTarget.Add(Asset::Manager::Get().GetMesh(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000002")),
							 Asset::Manager::Get().GetMaterial(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000000")));
        	AddChild(std::move(child));
        }

    private:
        String m_name;
    };
}

inline std::string to_string(const Coral::ECS::Entity& entity) {
	return entity.Name();
}
