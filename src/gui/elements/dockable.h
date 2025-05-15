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
    struct Dockable final : Element {
        explicit Dockable(
            String name,
            const Style& style,
            const std::initializer_list<Element*> children = {},
            std::function<void(Math::Vector2<f32>)> onResize = nullptr,
            ContextMenu* contextMenu = nullptr)
        : Element(style, children), m_name(std::move(name)), m_minSize(style.size), m_onResize(std::move(onResize)), m_contextMenu(contextMenu) {}

        ~Dockable() override = default;

        Dockable(const Dockable&) = delete;
        Dockable& operator=(const Dockable&) = delete;

		bool Render() override {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(m_minSize.width, m_minSize.height));
            ImGui::Begin(m_name.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);

            const auto size = Math::Vector2<f32>(ImGui::GetContentRegionAvail());
            const auto position = Math::Vector2<f32>(ImGui::GetCursorScreenPos());

            if (m_baseSize != size) {
                m_baseSize = size;
                m_position = position;
                ComputeLayout();

                if (m_onResize) {
                    m_onResize(size);
                }
            } else if (m_position != position) {
                SetPosition(position);
            }

			const String childName = "##" + m_name + "Child";
			ImGui::BeginChild(childName.c_str(), ImVec2(size.width, size.height), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			if (Element::Render()) {
				ComputeLayout();
			}
			ImGui::EndChild();
			if (m_contextMenu) {
				m_contextMenu->Render();
			}
			ImGui::End();

            ImGui::PopStyleVar();


			return false;
        }

    private:
        String m_name;
        Math::Vector2<f32> m_minSize;
        std::function<void(Math::Vector2<f32>)> m_onResize;
    	std::unique_ptr<ContextMenu> m_contextMenu;
    };
}
