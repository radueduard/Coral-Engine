//
// Created by radue on 2/13/2025.
//

#pragma once

#include <memory>
#include <vector>

#include "element.h"
#include "expanded.h"
#include "imgui.h"

namespace GUI {
    class Row : public Element {
    public:
        Row(const std::vector<Element*>& children, const float spacing)
            : m_spacing(spacing) {
            for (const auto &child : children) {
                m_children.emplace_back(child);
                child->AttachTo(this);
            }

            for (const auto &child : m_children) {
                if (typeid(*child) == typeid(Expanded)) {
                    m_numberOfExpanded++;
                }
            }
        }
        ~Row() override = default;

        void Render() override {
            m_outerBounds = m_parent->AllocatedArea(this);
            m_innerBounds.min = m_outerBounds.min;
            m_innerBounds.max = { m_outerBounds.min.x, m_outerBounds.max.y };

            m_requiredArea = { 0,  0 };
            for (const auto& child : m_children) {
                if (typeid(*child) != typeid(Expanded)) {
                    auto childRequiredArea = child->RequiredArea();
                    m_requiredArea.x += childRequiredArea.x;
                    m_requiredArea.y = std::max(m_requiredArea.y, childRequiredArea.y);
                }
            }
            m_requiredArea.x += static_cast<float>(m_children.size() - 1) * m_spacing;

            if (m_numberOfExpanded != 0) {
                const auto availableArea = m_outerBounds.max.x - m_outerBounds.min.x;
                m_expandedArea = { (availableArea - m_requiredArea.x) / static_cast<float>(m_numberOfExpanded), m_requiredArea.y };
                m_innerBounds.max = m_outerBounds.max;
            } else {
                m_innerBounds.max = { m_innerBounds.min.x + m_requiredArea.x, m_outerBounds.max.y };
            }

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(m_spacing, 0));

            for (const auto &child : m_children) {
                child->Render();
            }
            ImGui::PopStyleVar();
        }

        Math::Rect AllocatedArea(Element *element) const override {
            Math::Vector2<float> allocatedAreaStart = m_innerBounds.min;
            for (const auto &child : m_children) {
                if (element == child.get()) {
                    if (typeid(*child) != typeid(Expanded)) {
                        return { allocatedAreaStart, allocatedAreaStart + Math::Vector2(child->RequiredArea().x, m_innerBounds.max.y - m_innerBounds.min.y) };
                    }
                    return { allocatedAreaStart, allocatedAreaStart + m_expandedArea };
                }
                if (typeid(*child) != typeid(Expanded)) {
                    allocatedAreaStart.x += child->RequiredArea().x + m_spacing;
                } else {
                    allocatedAreaStart.x += m_expandedArea.x;
                }
            }
            return Math::Rect::Zero();
        }

    private:
        int m_numberOfExpanded = 0;
        Math::Vector2<float> m_expandedArea;

        std::vector<std::unique_ptr<Element>> m_children;
        float m_spacing;
    };
}