//
// Created by radue on 2/10/2025.
//

#pragma once

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gui/manager.h"

import types;
import math.rect;

using namespace Coral;

namespace GUI {
    class Element {
    public:
        Element() {
            m_id = generator();
        }
        explicit Element(const Math::Vector2<f32>& size) : m_requiredArea(size) {
            m_id = generator();
        }
        virtual ~Element() = default;

        Element(const Element&) = delete;
        Element& operator=(const Element&) = delete;

        virtual void Render() = 0;
        virtual Math::Rect AllocatedArea(Element *element) const { return Math::Rect::Zero(); }

        [[nodiscard]] const boost::uuids::uuid& Id() const { return m_id; }
        [[nodiscard]] std::string IdAsString() const { return to_string(m_id); }

        [[nodiscard]] const Element& Parent() const { return *m_parent; }
        void AttachTo(Element* parent) { m_parent = parent; }
        [[nodiscard]] const Math::Rect& InnerBounds() const { return m_innerBounds; }
        [[nodiscard]] const Math::Rect& OuterBounds() const { return m_outerBounds; }
        [[nodiscard]] const Math::Vector2<float>& RequiredArea() const { return m_requiredArea; }

        void Debug() const {
            const auto drawList = ImGui::GetWindowDrawList();
            drawList->AddRect(
                m_outerBounds.min + 0.5f,
                m_outerBounds.max - 0.5f,
                IM_COL32(0, 0, 255, 255)
            );

            drawList->AddRect(
                m_innerBounds.min - 0.5f,
                m_innerBounds.max + 0.5f,
                IM_COL32(255, 0, 0, 255)
            );
        }

    protected:
        inline static auto generator = boost::uuids::random_generator();
        boost::uuids::uuid m_id;

        Math::Rect m_outerBounds {};
        Math::Rect m_innerBounds {};

        Math::Vector2<f32> m_requiredArea = { 0.f, 0.f }; // 0 means as much as possible

        Element* m_parent = nullptr;
    };
}
