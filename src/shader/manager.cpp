//
// Created by radue on 2/7/2025.
//

#include "manager.h"

#include "gui/elements/popup.h"

namespace Coral::Shader {
	Manager::Manager(std::filesystem::path defaultSearchPath) : m_defaultSearchPath(std::move(defaultSearchPath)) {
		s_instance = this;
		m_currentPath = m_defaultSearchPath;
	}

	void Manager::Update() {
		for (auto& shader : m_shaders | std::views::values) {
			const auto lastTime = last_write_time(shader->Path());
			if (shader->LastWriteTime() != lastTime) {
				shader->Reload();
			} else {
				shader->m_changed = false;
			}
		}
	}
}
