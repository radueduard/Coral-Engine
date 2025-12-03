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
        Button(const Style& style, std::function<void()> onClick, const std::vector<Element*>& children = {})
         : Element(style, children), m_onClick(std::move(onClick)) {
             for (const auto& child : m_children) {
                 child->DisableInteraction();
             }
        }
        ~Button() override = default;

		void Subrender() override {
            const auto id = reinterpret_cast<const void*>(this);
            const auto outerBounds = ImRect {
                ImVec2 { m_actualRenderedPosition - Math::Vector2f(m_style.padding) / 2.f },
                ImVec2 { m_actualRenderedPosition + m_currentSize - Math::Vector2f(m_style.padding) / 2.f }
            };

            bool hovered, held;
            const bool clicked = ImGui::ButtonBehavior(outerBounds, ImGui::GetID(id), &hovered, &held,
                ImGuiButtonFlags_PressedOnRelease | ImGuiButtonFlags_AllowOverlap
            );

            ImU32 col;
            if (hovered) {
                col = ImGui::ColorConvertFloat4ToU32({ 1.f, 1.f, 1.f, .2f });
            } else {
                col = ImGui::ColorConvertFloat4ToU32({ 1.f, 1.f, 1.f, .0f });
            }

            ImGui::RenderNavHighlight(outerBounds, ImGui::GetID(id));
            ImGui::RenderFrame(outerBounds.Min, outerBounds.Max, col, true, m_style.cornerRadius);

            if (clicked) {
                m_onClick();
            }
        }
    private:
        std::function<void()> m_onClick;
    };
}
