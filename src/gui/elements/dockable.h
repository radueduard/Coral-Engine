//
// Created by radue on 2/10/2025.
//

#pragma once
#include <memory>
#include <string>
#include <utility>

#include "contextMenu.h"
#include "element.h"
#include "imgui.h"

namespace GUI {
    class Dockable final : public Element {
    public:
        Dockable(std::string  name, Element* child = nullptr, const Math::Vector2<float>& padding = 0.f, ContextMenu* contextMenu = nullptr)
            : m_name(std::move(name)), m_child(child), m_contextMenu(contextMenu), m_padding(padding) {
            if (m_child != nullptr) {
                m_child->AttachTo(this);
            }
        }
        ~Dockable() override = default;

        Dockable(const Dockable&) = delete;
        Dockable& operator=(const Dockable&) = delete;

        void Render() override {
            ImGui::Begin(m_name.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);

            if (m_contextMenu != nullptr)
                m_contextMenu->Show();

            const auto startPoint = ImGui::GetCursorScreenPos();
            const auto contentRegionAvail = ImGui::GetContentRegionAvail();

            const Math::Vector2 outerTopLeft = { startPoint.x, startPoint.y };
            const Math::Vector2 outerBottomRight = { startPoint.x + contentRegionAvail.x, startPoint.y + contentRegionAvail.y };

            const Math::Vector2 innerTopLeft = {
                startPoint.x + m_padding.x,
                startPoint.y + m_padding.y
            };

            const Math::Vector2 innerBottomRight = outerBottomRight - m_padding;

            m_outerBounds = Math::Rect { outerTopLeft, outerBottomRight };
            m_innerBounds = Math::Rect { innerTopLeft, innerBottomRight };

            if (m_child != nullptr) {
                m_child->Render();
            }

            ImGui::End();
        }

        Math::Rect AllocatedArea(Element *element) const override {
            if (element == m_child.get()) {
                return m_innerBounds;
            }
            return Math::Rect::Zero();
        }

        [[nodiscard]] const Element& Child() const { return *m_child; }

    private:
        std::string m_name;

        std::unique_ptr<Element> m_child;
        std::unique_ptr<ContextMenu> m_contextMenu = nullptr;

        Math::Vector2<float> m_padding = { 0.f, 0.f };
    };
}
