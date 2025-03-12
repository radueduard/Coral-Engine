//
// Created by radue on 2/10/2025.
//


#pragma once
#include <memory>

#include "element.h"

namespace GUI {
    class Expanded : public Element {
    public:
        explicit Expanded(Element* child = nullptr)
            : m_child(child) {
            if (m_child != nullptr) {
                child->AttachTo(this);
            }
        }
        ~Expanded() override = default;

        void Render() override {
            m_outerBounds = m_parent->AllocatedArea(this);
            m_innerBounds = m_outerBounds;

            if (m_child != nullptr) {
                m_child->Render();
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
    };
}
