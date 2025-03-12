//
// Created by radue on 2/22/2025.
//


#pragma once
#include "element.h"
#include "expanded.h"

namespace GUI {
    class Table final : public Element {
    public:
        Table(const std::vector<Element*>& children, const float spacing)
            : m_spacing(spacing) {
            for (const auto &child : children) {
                m_children.emplace_back(child);
                child->AttachTo(this);
            }
        }
        ~Table() override = default;

        void Render() override {
            m_outerBounds = m_parent->AllocatedArea(this);
            m_innerBounds.min = m_outerBounds.min;

            const float maxSpanX = m_outerBounds.max.x - m_outerBounds.min.x;
            m_requiredArea = { 0,  0 };
            int childrenOnCurrentRow = 0;
            Math::Vector2<float> currentRowArea = { 0, 0 };

            for (const auto& child : m_children ) {
                const auto childRect = child->RequiredArea();
                Math::Vector2<float> rowAreaWithChild = currentRowArea;
                rowAreaWithChild.x += childRect.x;
                rowAreaWithChild.y = std::max(rowAreaWithChild.y, childRect.y);
                if (rowAreaWithChild.x > maxSpanX) {
                    currentRowArea.x -= m_spacing;
                    m_requiredArea.x = std::max(m_requiredArea.x, currentRowArea.x);
                    m_requiredArea.y += currentRowArea.y + m_spacing;

                    childrenOnCurrentRow = 0;
                    currentRowArea = { 0, 0 };
                } else {
                    currentRowArea = rowAreaWithChild + Math::Vector2<float>(m_spacing, 0);
                    childrenOnCurrentRow++;
                }
            }
            if (childrenOnCurrentRow > 0) {
                m_requiredArea.x = std::max(m_requiredArea.x, currentRowArea.x);
                m_requiredArea.y += currentRowArea.y + m_spacing;
            }

            m_innerBounds.max = m_innerBounds.min + m_requiredArea;

            for (const auto &child : m_children) {
                child->Render();
            }
        }

        Math::Rect AllocatedArea(Element *element) const override {
            Math::Vector2<float> allocatedAreaStart = m_innerBounds.min;
            float rowHeight = 0;
            for (const auto &child : m_children) {
                auto childArea = child->RequiredArea();
                if (allocatedAreaStart.x + childArea.x < m_outerBounds.max.x) {
                    if (element == child.get()) {
                        return { allocatedAreaStart, allocatedAreaStart + childArea };
                    }
                } else {
                    allocatedAreaStart.y += rowHeight + m_spacing;
                    rowHeight = 0;
                    allocatedAreaStart.x = m_innerBounds.min.x;
                    if (element == child.get()) {
                        return { allocatedAreaStart, allocatedAreaStart + childArea };
                    }
                }
                rowHeight = std::max(rowHeight, childArea.y);
                allocatedAreaStart.x += childArea.x + m_spacing;
            }
            return Math::Rect::Zero();
        }

    private:
        std::vector<std::unique_ptr<Element>> m_children;
        float m_spacing;
    };
}
