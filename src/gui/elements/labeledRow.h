//
// Created by radue on 3/2/2025.
//

#pragma once

#include <memory>

#include "element.h"
#include "text.h"

namespace Coral::Reef {
	class LabeledRow final : public Element {
	public:
		LabeledRow(Text* label, Element* element, const Style& style = Style())
			: Element(style) {
			m_style.direction = Axis::Horizontal;
			AddChild(label);
			AddChild(element);
		}
		~LabeledRow() override = default;
    };
}
