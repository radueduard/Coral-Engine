//
// Created by radue on 2/24/2025.
//

#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>

#include "imgui.h"

namespace GUI {
	class ContextMenu {
	public:
		ContextMenu(std::unordered_map<std::string, std::function<void()>> actions) : m_actions(std::move(actions)) {}
		~ContextMenu() = default;

		void Show() {
			if (ImGui::BeginPopupContextWindow()) {
				for (const auto& [name, action] : m_actions) {
					if (ImGui::MenuItem(name.c_str())) {
						action();
					}
				}
				ImGui::EndPopup();
			}
		}

	private:
		std::unordered_map<std::string, std::function<void()>> m_actions;
	};
}
