//
// Created by radue on 5/14/2025.
//

#pragma once
#include "element.h"
#include "gui/manager.h"

namespace Coral::Reef {
	class Popup final : public Element {
	public:
		Popup(const String& name, const std::initializer_list<Element*>& children, const Style& style = Style { .direction = Axis::Vertical })
			: Element(style, children), m_name(name) {
			Context::GUIManager().RegisterPopup(name, this);
		}

		~Popup() override {
			Context::GUIManager().UnregisterPopup(m_name);
		}

		bool Render() override {
			if (!m_open) {
				return false;
			}

			ImGui::OpenPopup(m_name.c_str());
			ImGui::SetNextWindowSize(ImVec2(m_currentSize));

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, m_style.cornerRadius);
			if (ImGui::BeginPopupModal(m_name.c_str(), nullptr,
				ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::PopStyleVar();
				const auto screenPos = Math::Vector2<f32>(ImGui::GetCursorScreenPos());
				if (m_position != screenPos) {
					m_position = screenPos;
					SetPosition(m_position);
				}

				if (Element::Render()) {
					ComputeLayout();
				}
				ImGui::EndPopup();
				return false;
			}
			ImGui::PopStyleVar();
			return false;
		}

		void Open() {
			if (!m_open) {
				m_open = true;
				Element::ComputeLayout();
			}
		}

		void Close() {
			if (m_open) {
				m_open = false;
				ImGui::CloseCurrentPopup();
			}
		}

	private:
		bool m_open = false;
		String m_name;
	};
}