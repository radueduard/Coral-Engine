//
// Created by radue on 2/10/2025.
//


#pragma once
#include <glm/glm.hpp>
#include <iostream>

namespace GUI {
    class Element {
    public:
        Element() = default;
        explicit Element(std::string name, const glm::vec2 size = { 0.f, 0.f })
            : m_allocatedArea(size), m_name(std::move(name)) {}
        virtual ~Element() = default;

        Element(const Element&) = delete;
        Element& operator=(const Element&) = delete;

        virtual void Render() = 0;
        virtual glm::vec2 StartPoint(Element* element) { return {}; }
        virtual glm::vec2 AllocatedArea(Element* element) { return {}; }

        [[nodiscard]] const std::string& Name() const { return m_name; }
        [[nodiscard]] glm::vec2 Size() const { return m_allocatedArea; }
        [[nodiscard]] float Width() const { return m_allocatedArea.x; }
        [[nodiscard]] float Height() const { return m_allocatedArea.y; }


        [[nodiscard]] const Element& Parent() const { return *m_parent; }
        void AttachTo(Element* parent) { m_parent = parent; }

        void Debug() { std::cout << typeid(*this).name() << ": " << std::endl <<
                "\tAvailable area: " << m_availableArea.x << " x " <<  m_availableArea.y << std::endl <<
                "\tAllocated area: " << m_allocatedArea.x << " x " <<  m_allocatedArea.y << std::endl <<
                "\tStart point: " << m_startPoint.x << " x " << m_startPoint.y << std::endl <<
                    std::endl; }

    protected:
        glm::vec2 m_startPoint {};
        glm::vec2 m_availableArea {};
        glm::vec2 m_allocatedArea {};
        float m_numberOfExpanded = 0.f;

        Element* m_parent = nullptr;
    private:
        std::string m_name;
    };
}