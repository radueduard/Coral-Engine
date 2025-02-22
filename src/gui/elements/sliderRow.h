//
// Created by radue on 2/20/2025.
//


#pragma once
#include <memory>
#include <string>

#include "element.h"
#include "row.h"
#include "slider.h"
#include "text.h"

namespace GUI {

	template <typename T, int N, typename = std::enable_if_t<std::is_arithmetic_v<T> && N >= 1 && N <= 4>>
	class SliderRow final : public Element {
	public:
		SliderRow(const std::string &name, T** value, T min, T max, T step = 0.1f) {
			m_element = std::make_unique<Row>(
				std::vector<Element*> {
					new GUI::Expanded(
						new GUI::Center(
							new GUI::Text(
								name,
								GUI::Text::Style {
									Math::Color { 1.f, 1.f, 1.f, 1.f },
									15.f,
									GUI::Bold
								}
							),
							false,
							true
						)
					),
					new GUI::Slider<T, N>("##" + name, value, min, max, step)
				},
				10.f
			);
			m_element->AttachTo(this);
		}
		~SliderRow() override = default;

		void Render() override {
			m_outerBounds = m_parent->AllocatedArea(this);
			m_innerBounds = m_outerBounds;
			m_requiredArea = m_element->RequiredArea();

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
