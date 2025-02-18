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
            m_startPoint = m_parent->StartPoint(this);
            m_availableArea = m_parent->AllocatedArea(this);
            m_allocatedArea = m_child->Size();

            m_child->Render();
        }

        glm::vec2 StartPoint(Element *element) override {
            // Debug();

            if (m_horizontal && m_vertical) {
                return m_startPoint + glm::vec2 { (m_availableArea.x - m_allocatedArea.x) / 2, (m_availableArea.y - m_allocatedArea.y) / 2 };
            } if (m_horizontal) {
                return m_startPoint + glm::vec2 { (m_availableArea.x - m_allocatedArea.x) / 2, 0.f };
            } if (m_vertical) {
                return m_startPoint + glm::vec2 { 0.f, (m_availableArea.y - m_allocatedArea.y) / 2 };
            }
            throw std::runtime_error("How did we get here?");
        }

        glm::vec2 AllocatedArea(Element *element) override {
            return m_allocatedArea;
        }

    private:
        std::unique_ptr<Element> m_child;
        bool m_horizontal;
        bool m_vertical;
    };
}
