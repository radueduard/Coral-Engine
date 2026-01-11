//
// Created by radue on 6/25/2025.
//

#include "graphics/objects/material.h"
#include "scene.h"
#include "gui/container.h"
#include "sceneManager.h"
#include "ecs/entity.h"

Coral::ECS::SceneManager::SceneManager() {
	static bool firstTime = true;
	if (!firstTime) {
		throw std::runtime_error("SceneManager has already been initialised!");
	}
	firstTime = false;
	Context::m_sceneManager = this;

	m_registry = entt::registry();

	NewScene();
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


