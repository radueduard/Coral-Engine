//
// Created by radue on 6/25/2025.
//

#include "sceneManager.h"
#include "ecs/entity.h"
#include "gui/elements/popup.h"

Coral::ECS::SceneManager::SceneManager() {
	s_instance = this;
	m_registry = entt::registry();
	NewScene();
}
Coral::ECS::SceneManager& Coral::ECS::SceneManager::Get() {
	if (s_instance == nullptr) {
		throw std::runtime_error("SceneManager is not initialized.");
	}
	return *s_instance;
}
Coral::ECS::Scene& Coral::ECS::SceneManager::GetLoadedScene() const {
	if (m_loadedScene == nullptr) {
		throw std::runtime_error("No scene is currently loaded.");
	}
	return *m_loadedScene;
}
bool Coral::ECS::SceneManager::IsSceneLoaded() const { return m_loadedScene != nullptr; }
void Coral::ECS::SceneManager::NewScene() {
	m_loadedScene.reset();
	m_registry = entt::registry();
	m_loadedScene = Reef::MakeContainer<Scene>();
	m_loadedScene->Setup();
}
void Coral::ECS::SceneManager::Update(float deltaTime) {
	for (const auto& event : m_events) {
		event();
	}
	m_events.clear();
}
void Coral::ECS::SceneManager::RegisterEvent(std::function<void()> event) { m_events.emplace_back(std::move(event)); }


