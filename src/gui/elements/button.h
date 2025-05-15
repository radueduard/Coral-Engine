//
// Created by radue on 2/10/2025.
//

#pragma once
#include <functional>
#include <memory>
#include <utility>

#include "element.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace Coral::Reef {
    class Button final : public Element {
    public:
        Button(const Style& style, std::function<bool()> onClick, const std::initializer_list<Element*> children = {})
         : Element(style, children), m_onClick(std::move(onClick)) {}
        ~Button() override = default;

		bool Render() override {
			const bool shouldReset = Element::Render();

            ImGui::PushID(this);
            const auto id = "Button";
            const auto outerBounds = ImRect {
                { m_position.x, m_position.y },
                { m_position.x + m_currentSize.width, m_position.y + m_currentSize.height }
            };

            bool hovered, held;
            const bool clicked = ImGui::ButtonBehavior(outerBounds, ImGui::GetID(id), &hovered, &held, ImGuiButtonFlags_PressedOnRelease);

            ImU32 col;
            if (hovered) {
                col = ImGui::ColorConvertFloat4ToU32({ 1.f, 1.f, 1.f, .2f });
            } else {
                col = ImGui::ColorConvertFloat4ToU32({ 1.f, 1.f, 1.f, .0f });
            }

            ImGui::RenderNavHighlight(outerBounds, ImGui::GetID(id));
            ImGui::RenderFrame(outerBounds.Min, outerBounds.Max, col, true, m_cornerRadius);
            ImGui::PopID();

            if (clicked) {
                return m_onClick() || shouldReset;
            }
			return shouldReset;
        }
    private:
        std::function<bool()> m_onClick;
    };
}
