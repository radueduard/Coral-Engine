//
// Created by radue on 2/7/2025.
//

#include "manager.h"

#include "gui/elements/popup.h"

namespace Coral::Shader {
	Manager::Manager(std::filesystem::path defaultSearchPath) : m_defaultSearchPath(std::move(defaultSearchPath)) {
		s_instance = this;
		m_currentPath = m_defaultSearchPath;
		m_shaderStorage = std::make_unique<Slang>();
	}

	void Manager::Update() const {
		for (const auto& shader : m_shaderStorage->modules | std::views::values) {
			for (const auto& entryPoint : shader->entryPoints | std::views::values) {
				entryPoint->Update();
			}
		}
	}

	void Manager::LateUpdate() const {
		for (const auto& shader : m_shaderStorage->modules | std::views::values) {
			for (const auto& entryPoint : shader->entryPoints | std::views::values) {
				entryPoint->LateUpdate();
			}
		}
	}
}
