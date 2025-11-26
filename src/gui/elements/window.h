//
// Created by radue on 2/10/2025.
//

#pragma once

#include <string>
#include <utility>

#include "contextMenu.h"
#include "element.h"
#include "imgui.h"

namespace Coral::Reef {
    struct Window final : Element {
        explicit Window(
            String name,
            const Style& style = {},
            const std::initializer_list<Element*> children = {},
            std::function<void(Math::Vector2<f32>)> onResize = nullptr,
            ContextMenu* contextMenu = nullptr)
        : Element(style, children), m_name(std::move(name)), m_onResize(std::move(onResize)) {}

        ~Window() override = default;

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

		bool Render() override {
		    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(m_style.padding.left, m_style.padding.top));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, m_style.cornerRadius);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(m_minSize.width, m_minSize.height));

			ImGui::PushStyleColor(ImGuiCol_WindowBg, static_cast<ImVec4>(m_style.backgroundColor));

            ImGui::Begin(m_name.c_str(), nullptr,
            	ImGuiWindowFlags_NoCollapse |
            	ImGuiWindowFlags_NoTitleBar |
            	ImGuiWindowFlags_NoScrollbar
            );

			auto siz = ImGui::GetWindowSize();
            const auto size = Math::Vector2f(ImGui::GetContentRegionAvail());
            const auto position = Math::Vector2f(ImGui::GetCursorScreenPos());

            if (m_baseSize != size) {
                m_baseSize = size;
                m_position = position;

            	ComputeLayout();

                if (m_onResize) {
                    m_onResize(size);
                }
            } else if (RecreateRequired()) {
				ComputeLayout();
            } else if (m_position != position) {
                SetPosition(position);
            }

			const String childName = "##" + boost::uuids::to_string(m_uuid);
			ImGui::BeginChild(childName.c_str(), ImVec2(size.width, size.height), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

			bool shouldReset = false;
			for (const auto& child : m_children) {
				shouldReset |= child->Render();
			}

		    if (shouldReset) {
				ComputeLayout();
		    }

			if (m_contextMenu) {
				m_contextMenu->Render();
			}

			ImGui::EndChild();

			for (auto& popup : m_popups) {
				if (popup->Render()) {
					ComputeLayout();
				}
			}

			ImGui::End();


			ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);



			return false;
        }

    	void AddPopups(const std::vector<Element*>& popups) {
			for (auto* popup : popups) {
				m_popups.emplace_back(std::unique_ptr<Element>(popup));
			}
		}

    private:
        String m_name;
        std::function<void(Math::Vector2<f32>)> m_onResize;
    	std::unique_ptr<ContextMenu> m_contextMenu;
    	std::vector<std::unique_ptr<Element>> m_popups;
    };
}
