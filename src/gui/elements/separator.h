//
// Created by radue on 2/18/2025.
//

#pragma once

#include "element.h"
#include "imgui.h"

namespace GUI {
	class Separator : public Element {
	public:
		Separator() = default;
		~Separator() override = default;

		void Render() override {
			m_startPoint = m_parent->StartPoint(this);
			m_availableArea = m_parent->AllocatedArea(this);
			m_allocatedArea = { 0, m_size + 2 * ImGui::GetStyle().SeparatorTextBorderSize };

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::Separator();
			ImGui::PopStyleVar();
		}
	private:
		float m_size = 1.f;
    };
}
