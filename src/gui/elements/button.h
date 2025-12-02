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

        void Render() override {
            if (m_style.debug) {
                Debug();
            }

            auto cornerRadius = m_style.cornerRadius;
            if (const auto smallerDimension = std::min(m_currentSize.width, m_currentSize.height);
                cornerRadius * 2 > smallerDimension)
            {
                cornerRadius = smallerDimension / 2.f;
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(m_style.padding.left, m_style.padding.top));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, cornerRadius);
            ImGui::PushStyleColor(
                ImGuiCol_ChildBg,
                ImGui::ColorConvertFloat4ToU32(ImVec4(m_style.backgroundColor))
            );

            ImGui::SetCursorPos({ m_relativePosition.x, m_relativePosition.y });
            ImGui::BeginChild(
                boost::uuids::to_string(m_uuid).c_str(),
                ImVec2(m_currentSize.width, m_currentSize.height),
                ImGuiChildFlags_AlwaysUseWindowPadding,
                ImGuiWindowFlags_NoDecoration
            );

            m_actualRenderedPosition = Math::Vector2<f32>(ImGui::GetCursorScreenPos());

            Subrender();
            ImGui::SetCursorScreenPos({ m_actualRenderedPosition.x, m_actualRenderedPosition.y });
            for (const auto& child : m_children) {
                child->Render();
            }

            ImGui::EndChild();


            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);

        }

		void Subrender() override {
            const auto id = reinterpret_cast<const void*>(this);
            const auto outerBounds = ImRect {
                { m_actualRenderedPosition.x, m_actualRenderedPosition.y },
                { m_actualRenderedPosition.x + m_currentSize.width, m_actualRenderedPosition.y + m_currentSize.height }
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
