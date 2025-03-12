//
// Created by radue on 2/10/2025.
//


#pragma once
#include <memory>

#include "element.h"
#include "imgui.h"

namespace GUI {
    class Center : public Element {
    public:
        explicit Center(Element* child, const bool horizontal = true, const bool vertical = true)
            : m_child(child), m_horizontal(horizontal), m_vertical(vertical) {
            if (!(m_horizontal || m_vertical)) throw std::runtime_error("Why even use center?");
            child->AttachTo(this);
        }
        ~Center() override = default;

        void Render() override {
            m_requiredArea = m_child->RequiredArea();
            m_outerBounds = m_parent->AllocatedArea(this);
            const auto childArea = m_child->RequiredArea();
            if (m_horizontal && !m_vertical) {
                m_innerBounds.min.x = m_outerBounds.min.x + (m_outerBounds.max.x - m_outerBounds.min.x - childArea.x) / 2.0f;
                m_innerBounds.max.x = m_innerBounds.min.x + childArea.x;

                m_innerBounds.min.y = m_outerBounds.min.y;
                m_innerBounds.max.y = m_outerBounds.max.y;
            } else if (m_vertical && !m_horizontal) {
                m_innerBounds.min.y = m_outerBounds.min.y + (m_outerBounds.max.y - m_outerBounds.min.y - childArea.y) / 2.0f;
                m_innerBounds.max.y = m_innerBounds.min.y + childArea.y;

                m_innerBounds.min.x = m_outerBounds.min.x;
                m_innerBounds.max.x = m_outerBounds.max.x;
            } else {
                m_innerBounds.min = m_outerBounds.min + (m_outerBounds.max - m_outerBounds.min - childArea) / 2.0f;
                m_innerBounds.max = m_innerBounds.min + childArea;
            }

            m_child->Render();
        }

        Math::Rect AllocatedArea(Element *element) const override {
            if (element == m_child.get()) {
                return m_innerBounds;
            }
            return Math::Rect::Zero();
        }

    private:
        std::unique_ptr<Element> m_child;
        bool m_horizontal;
        bool m_vertical;
    };
}
