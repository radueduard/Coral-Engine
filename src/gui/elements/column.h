//
// Created by radue on 2/13/2025.
//

#pragma once

#pragma once
#include <memory>
#include <string>
#include <vector>

#include "element.h"
#include "expanded.h"
#include "imgui.h"

namespace GUI {
    class Column : public Element {
    public:
        Column(std::string name, const std::vector<Element*>& children, const float spacing)
            : Element(std::move(name)), m_spacing(spacing) {
            for (const auto &child : children) {
                m_children.emplace_back(child);
                child->AttachTo(this);
            }

            for (const auto &child : m_children) {
                if (typeid(*child) == typeid(Expanded)) {
                    m_numberOfExpanded++;
                } else {
                    m_allocatedArea.x = std::max(m_allocatedArea.x, child->Width());
                }
            }
        }
        ~Column() override = default;

        void Render() override {
            m_startPoint = m_parent->StartPoint(this);
            m_availableArea = m_parent->AllocatedArea(this);
            m_allocatedArea = { m_availableArea.x, 0 };

            for (const auto& child : m_children) {
                if (typeid(*child) != typeid(Expanded)) {
                    m_allocatedArea.y += child->Height();
                }
            }
            m_allocatedArea.y += static_cast<float>(m_children.size() - 1) * m_spacing;

            if (m_numberOfExpanded != 0) {
                m_allocatedArea.x = m_availableArea.x;
            }

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, m_spacing));

            for (const auto &child : m_children) {
                child->Render();
            }
            ImGui::PopStyleVar();

        }

        glm::vec2 StartPoint(Element *element) override {
            glm::vec2 offset = m_startPoint;
            for (const auto& child : m_children) {
                if (child.get() == element) {
                    return offset;
                }
                offset += glm::vec2 { 0.f, child->Height() + m_spacing };
            }

            throw std::runtime_error("There is no area allocated for elements that are not children");
        }

        glm::vec2 AllocatedArea(Element *element) override {
            for (const auto& child : m_children) {
                if (element == child.get()) {
                    if (typeid(*element) == typeid(Expanded)) {
                        return { m_availableArea.x, (m_availableArea.y - m_allocatedArea.y) / m_numberOfExpanded };
                    }
                    return { m_allocatedArea.x,  child->Height() };
                }
            }
            throw std::runtime_error("There is no area allocated for elements that are not children");
        }

    private:
        std::vector<std::unique_ptr<Element>> m_children;
        float m_spacing;
    };
}