//
// Created by radue on 6/25/2025.
//

#pragma once

#include "gui/container.h"
#include "scene.h"

namespace Coral::ECS {
	class SceneManager {
	public:
		SceneManager();
		~SceneManager() = default;

		static SceneManager& Get();

		Scene& GetLoadedScene() const;

		bool IsSceneLoaded() const;

		void NewScene();

		entt::registry& Registry() {
			return m_registry;
		}

		void Update(float deltaTime);

		void RegisterEvent(std::function<void()> event);

	private:
		std::vector<std::function<void()>> m_events;
		inline static SceneManager* s_instance = nullptr;

        entt::registry m_registry;
		Reef::Container<Scene> m_loadedScene = nullptr;
	};
}
