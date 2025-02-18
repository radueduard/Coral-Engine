//
// Created by radue on 2/10/2025.
//


#pragma once
#include <functional>
#include <string>

#include "element.h"
#include "imgui.h"

namespace GUI {
    class IconButton : public Element {
    public:
        IconButton(std::string name, std::string icon, std::function<void()> onClick, const glm::uvec2 size = { 50, 50 })
            : Element(std::move(name), size), m_icon(std::move(icon)), m_onClick(std::move(onClick)) {}
        ~IconButton() override = default;

        void Render() override {
            m_startPoint = m_parent->StartPoint(this);

            ImGui::SetCursorPos({ m_startPoint.x, m_startPoint.y });

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            if (ImGui::Button(m_icon.c_str(), ImVec2(m_allocatedArea.x, m_allocatedArea.y))) {
                m_onClick();
            }

            ImGui::PopStyleVar();
        }

        glm::vec2 StartPoint(Element *element) override {
            return { 0, 0 };
        }

        glm::vec2 AllocatedArea(Element *element) override {
            return { 0, 0 };
        }

    private:
        std::string m_icon;
        std::function<void()> m_onClick;
    };
}
