//
// Created by radue on 2/10/2025.
//

#pragma once
#include <memory>
#include <string>
#include <utility>

#include "element.h"
#include "imgui.h"

namespace GUI {
    class Dockable : public Element {
    public:
        Dockable(std::string name, Element* child, const glm::vec2 padding)
            : Element(std::move(name)), m_child(child), m_padding(padding) {
            child->AttachTo(this);
        }
        ~Dockable() override = default;

        Dockable(const Dockable&) = delete;
        Dockable& operator=(const Dockable&) = delete;

        void Render() override {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
            ImGui::Begin(Name().c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
            auto windowSize = ImGui::GetWindowSize();
            auto contentRegionAvail = ImGui::GetContentRegionAvail();
            m_availableArea = { windowSize.x, windowSize.y };
            m_allocatedArea = { contentRegionAvail.x, contentRegionAvail.y };

            m_child->Render();

            ImGui::End();
            ImGui::PopStyleVar();
        }

        glm::vec2 StartPoint(Element *element) override {
            return glm::vec2 { m_availableArea.x - m_allocatedArea.x, m_availableArea.y - m_allocatedArea.y } + m_padding;
        }

        glm::vec2 AllocatedArea(Element *element) override {
            if (element == m_child.get()) {
                return m_allocatedArea - 2.f * m_padding;
            }
            throw std::runtime_error("There is no area allocated for elements that are not children");
        }

        [[nodiscard]] const Element& Child() const { return *m_child; }

    private:
        std::unique_ptr<Element> m_child;
        glm::vec2 m_padding = { 0.f, 0.f };
    };
}
