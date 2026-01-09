//
// Created by radue on 5/10/2025.
//

#pragma once
#include <functional>

#include "element.h"

namespace Coral::Reef {
	class Conditional final : public Element {
	public:
		explicit Conditional(
			const std::function<u8()>& condition,
			bool* changed,
			const std::vector<Element*>& children)
		: m_condition(condition), m_changed(changed) {
			for (auto* child : children) {
				m_elements.emplace_back(child);
			}
			m_state = m_condition();
			AddChild(m_elements[m_state].release());
			m_lastState = m_state;
		}

		void Update() override {
			Element::Update();
			m_state = m_condition();
			if (m_state != m_lastState) {
				ResetState([this] {
					auto oldElement = std::move(m_children.front());
					m_children.clear();
					m_elements[m_lastState] = std::move(oldElement);
					AddChild(m_elements[m_state].release());
				});
				m_lastState = m_state;
				*m_changed = true;
			}
		}

	private:
		u8 m_state;
		bool* m_changed;
		u8 m_lastState;
		std::vector<std::unique_ptr<Element>> m_elements;
		std::function<u8()> m_condition;
	};
}
