//
// Created by radue on 3/6/2025.
//

#pragma once

#include "element.h"

namespace GUI {
	class Scrollable final : public Element {
	public:
		explicit Scrollable(Element* child) : m_child(child) {
			child->AttachTo(this);
		}

		void Render() override {
			m_outerBounds = m_parent->AllocatedArea(this);
			m_innerBounds = m_child->InnerBounds();
			m_requiredArea = m_child->RequiredArea();

			ImGui::SetCursorScreenPos(m_innerBounds.min);

			ImGui::BeginChild("scrollable", m_requiredArea, true, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
			if (m_child) {
				m_child->Render();
			}
			ImGui::EndChild();
		}

		Math::Rect AllocatedArea(Element *element) const override {
			if (element == this) {
				return m_outerBounds;
			}
			return Math::Rect::Zero();
		}

	private:
		std::unique_ptr<Element> m_child;
    };
}