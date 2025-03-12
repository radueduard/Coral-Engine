//
// Created by radue on 2/10/2025.
//


#pragma once
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "element.h"
#include "imgui.h"

namespace GUI {
    class Button final : public Element {
    public:
        Button(std::string icon, std::function<void()> onClick, const Math::Vector2<float> size)
            : m_icon(std::move(icon)), m_onClick(std::move(onClick)) {
            m_requiredArea = size;
        }
        ~Button() override = default;

        void Render() override {
            const auto minSizeRequired = Math::Vector2<float>(ImGui::CalcTextSize(m_icon.c_str())) +
                                            Math::Vector2<float>(ImGui::GetStyle().FramePadding) * 2.0f;
            if (minSizeRequired.x > m_requiredArea.x) {
                m_requiredArea.x = minSizeRequired.x;
            }
            if (minSizeRequired.y > m_requiredArea.y) {
                m_requiredArea.y = minSizeRequired.y;
            }

            m_outerBounds = m_parent->AllocatedArea(this);
            m_innerBounds = m_outerBounds;

            ImGui::SetCursorScreenPos(m_innerBounds.min);

            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.5f, 0.5f });
            ImGui::PushFont(g_manager->GetFont(FontType::Black, 20.0f));

            if (ImGui::ButtonEx(m_icon.c_str(), m_requiredArea, ImGuiButtonFlags_AlignTextBaseLine)) {
                m_onClick();
            }

            ImGui::PopFont();
            ImGui::PopStyleVar();
        }

    private:
        std::string m_icon;
        std::function<void()> m_onClick;
    };

    class ButtonArea final : public Element {
    public:
        ButtonArea(Element* child, const std::function<void()> &onClick, const Math::Vector2<float> padding)
            : m_child(child), m_onClick(onClick), m_padding(padding) {
            child->AttachTo(this);
        }
        ~ButtonArea() override = default;

        void Render() override {
            m_outerBounds = m_parent->AllocatedArea(this);
            m_innerBounds = { m_outerBounds.min + m_padding, m_outerBounds.max - m_padding };
            m_requiredArea = m_child->RequiredArea() + m_padding * 2.0f;

            ImGui::SetCursorScreenPos(m_innerBounds.min);

            const auto id = m_child->IdAsString() + "Button";

            bool hovered, held;
            const bool clicked = ImGui::ButtonBehavior(m_outerBounds, ImGui::GetID(id.c_str()), &hovered, &held, ImGuiButtonFlags_PressedOnClick);

            const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
            ImGui::RenderNavHighlight(m_outerBounds, ImGui::GetID(id.c_str()));
            ImGui::RenderFrame(m_outerBounds.min, m_outerBounds.max, col, true, ImGui::GetStyle().FrameRounding);

            if (m_child != nullptr) {
                m_child->Render();
            }

            if (clicked) {
                m_onClick();
            }
        }

        Math::Rect AllocatedArea(Element *element) const override {
            if (element == m_child.get()) {
                return m_innerBounds;
            }
            return Math::Rect::Zero();
        }

    private:
        std::unique_ptr<Element> m_child;
        std::function<void()> m_onClick;
        Math::Vector2<float> m_padding;
    };
}
