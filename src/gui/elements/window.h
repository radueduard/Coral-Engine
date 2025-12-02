//
// Created by radue on 2/10/2025.
//

#pragma once

#include <string>
#include <utility>

#include "element.h"
#include "imgui.h"

namespace Coral::Reef {
    struct Window final : Element {
        explicit Window(
            String name,
            const Style& style = {},
            const std::vector<Element*>& children = {},
            std::function<void(Math::Vector2<f32>)> onResize = nullptr)
        : Element(style, children), m_name(std::move(name)), m_onResize(std::move(onResize)) {}

        ~Window() override = default;

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        void Update() override
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, m_style.cornerRadius);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(m_minSize.width, m_minSize.height));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, static_cast<ImVec4>(m_style.backgroundColor));
            ImGui::Begin(m_name.c_str(), nullptr,
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoScrollbar
            );

            const auto size = Math::Vector2f(ImGui::GetContentRegionAvail());
            if (m_baseSize != size) {
                m_baseSize = size;
                if (m_onResize) {
                    m_onResize(size);
                }
                m_shouldResize = true;
            }

            const auto position = Math::Vector2f(ImGui::GetCursorScreenPos());
            if (m_absolutePosition != position) {
                m_absolutePosition = position;
                m_shouldResize = true;
            }

			ImGui::End();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);

            bool extraReason = false;
            for (const auto& child : m_children) {
                child->Update();
                if (child->RecreateRequired()) {
                    extraReason = true;
                    break;
                }
            }

            if (RecreateRequired() || extraReason) {
                ComputeLayout();
            }
        }

		void Render() override {
		    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(m_style.padding.left, m_style.padding.top));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, m_style.cornerRadius);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(m_minSize.width, m_minSize.height));
			ImGui::PushStyleColor(ImGuiCol_WindowBg, static_cast<ImVec4>(m_style.backgroundColor));

            ImGui::Begin(m_name.c_str(), nullptr,
            	ImGuiWindowFlags_NoCollapse |
            	ImGuiWindowFlags_NoTitleBar |
            	ImGuiWindowFlags_NoScrollbar
            );

			const String childName = "##" + boost::uuids::to_string(m_uuid);
			ImGui::BeginChild(
			    childName.c_str(),
			    ImVec2(m_baseSize),
			    false,
			    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        	for (const auto& child : m_children) {
				child->Render();
			}

			ImGui::EndChild();
			ImGui::End();

			ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);
        }

    private:
        String m_name;
        std::function<void(Math::Vector2<f32>)> m_onResize;
    };
}


