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
    class Column final : public Element {
    public:
        Column(const std::vector<Element*>& children, const float spacing) : m_spacing(spacing) {
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
        ~Column() override = default;

        void Render() override {
            m_outerBounds = m_parent->AllocatedArea(this);
            m_innerBounds.min = m_outerBounds.min;
            m_innerBounds.max = { m_outerBounds.max.x, m_outerBounds.min.y };

            m_requiredArea = { 0, 0 };
            for (const auto& child : m_children) {
                if (typeid(*child) != typeid(Expanded)) {
                    m_requiredArea.y += child->RequiredArea().y;
                    m_requiredArea.x = std::max(m_requiredArea.x, child->RequiredArea().x);
                }
            }
            m_requiredArea.y += static_cast<float>(m_children.size() - 1) * m_spacing;

            if (m_numberOfExpanded != 0) {
                const auto availableArea = m_outerBounds.max.y - m_outerBounds.min.y;
                m_expandedArea = { m_requiredArea.x, (availableArea - m_requiredArea.y) / static_cast<float>(m_numberOfExpanded) };
                m_innerBounds.max = m_outerBounds.max;
            } else {
                m_innerBounds.max = { m_outerBounds.max.x, m_innerBounds.min.y + m_requiredArea.y };
            }

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, m_spacing));

            for (const auto &child : m_children) {
                child->Render();
            }
            ImGui::PopStyleVar();
        }

        Math::Rect AllocatedArea(Element *element) const override {
            Math::Vector2<float> allocatedAreaStart = m_innerBounds.min;
            for (const auto& child : m_children) {
                if (element == child.get()) {
                    if (typeid(*element) != typeid(Expanded)) {
                        return { allocatedAreaStart, allocatedAreaStart + Math::Vector2(m_innerBounds.max.x - m_innerBounds.min.x, child->RequiredArea().y) };
                    }
                    return { allocatedAreaStart, allocatedAreaStart + m_expandedArea };
                }
                if (typeid(*child) != typeid(Expanded)) {
                    allocatedAreaStart.y += child->RequiredArea().y + m_spacing;
                } else {
                    allocatedAreaStart.y += m_expandedArea.y + m_spacing;
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