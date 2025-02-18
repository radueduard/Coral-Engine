//
// Created by radue on 2/10/2025.
//


#pragma once
#include <memory>

#include "element.h"

namespace GUI {
    class Expanded : public Element {
    public:
        explicit Expanded(Element* child)
            : m_child(child) {
            child->AttachTo(this);
        }
        ~Expanded() override = default;

        void Render() override {
            m_startPoint = m_parent->StartPoint(this);
            m_allocatedArea = m_parent->AllocatedArea(this);

            m_child->Render();
        }

        glm::vec2 StartPoint(Element *element) override {
            return m_parent->StartPoint(this);
        }

        glm::vec2 AllocatedArea(Element *element) override {
            return m_parent->AllocatedArea(this);
        }

    private:
        std::unique_ptr<Element> m_child;
    };
}
