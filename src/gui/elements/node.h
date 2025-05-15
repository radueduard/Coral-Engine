//
// Created by radue on 5/12/2025.
//

#pragma once

#include "element.h"

#include <imgui.h>
#include <imnodes.h>

#include <utility>

namespace Coral::Reef {
	class Node final : public Element {
	public:
		explicit Node(std::string  name, const Style& style = Style())
			: Element(style), m_id(rand() % 1000000), m_name(std::move(name)) {}
		~Node() override = default;

		bool Render() override {

			ImNodes::BeginNode(m_id);
			ImGui::Dummy(ImVec2(m_currentSize));
			ImNodes::EndNode();
		}

	private:
		int m_id = 0;
		std::string m_name;
	};
}
