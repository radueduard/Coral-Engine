//
// Created by radue on 3/2/2025.
//

#pragma once

#include <memory>

#include "center.h"
#include "element.h"
#include "expanded.h"
#include "row.h"
#include "text.h"
#include "gui/manager.h"

namespace GUI {
	class LabeledRow final : public Element {
	public:
		LabeledRow(const std::string &name, Element* element) {
			m_element = std::make_unique<Row>(
				std::vector<Element*> {
					new GUI::Center(
						new GUI::Text(
							name,
							GUI::Text::Style {
								Math::Color { 1.f, 1.f, 1.f, 1.f },
								15.f,
								FontType::Bold
							}
						),
						false,
						true
					),
					new Expanded(),
					element
				},
				10.f
			);
			m_element->AttachTo(this);
		}
		~LabeledRow() override = default;

		void Render() override {
			m_outerBounds = m_parent->AllocatedArea(this);
			m_innerBounds = m_outerBounds;
			m_requiredArea = m_element->RequiredArea();

			ImGui::SetCursorScreenPos(m_innerBounds.min);

			m_element->Render();
		}

		Math::Rect AllocatedArea(Element *element) const override {
			if (element == m_element.get()) {
				return m_innerBounds;
			}
			return Math::Rect::Zero();
		}
	private:
		std::unique_ptr<Element> m_element;
    };
}
