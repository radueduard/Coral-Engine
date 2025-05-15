//
// Created by radue on 5/12/2025.
//

#pragma once

#include "element.h"

#include <imgui.h>
#include <imnodes.h>

#include "node.h"

namespace Coral::Reef {
	class NodeSpace final : public Element {
	public:
		explicit NodeSpace(const std::vector<Node*>& nodes, const Style& style = Style())
			: Element(style) {
			for (auto node : nodes) {
				this->m_children.emplace_back(node);
			}
		}
		~NodeSpace() override = default;

		bool Render() override {
			ImNodes::BeginNodeEditor();
			Element::Render();
			ImNodes::EndNodeEditor();
		}
	};
}