//
// Created by radue on 2/18/2025.
//

#pragma once

#include <vector>

#include "element.h"
#include "imgui.h"

namespace GUI {
	class Text final : public Element {
	public:
		struct Style {
			Math::Color color = { 1.f, 1.f, 1.f, 1.f };
			float fontSize = 13.f;
			FontType fontType = FontType::Regular;
		};

		class Builder {
			friend class Text;
		public:
			Builder& AddText(std::string text, Style style = {}) {
				m_text.emplace_back(std::move(text), std::move(style));
				return *this;
			}

			std::unique_ptr<Text> Build() {
				return std::make_unique<Text>(this);
			}

		private:
			std::vector<std::pair<std::string, Style>> m_text {};
		};

		~Text() override = default;

		explicit Text(std::string text, Style style = {}) {
			m_text.emplace_back(std::move(text), std::move(style));
		}

		explicit Text(Builder* builder) {
			m_text = std::move(builder->m_text);
		}

		void Render() override {
			m_outerBounds = m_parent->AllocatedArea(this);
			m_innerBounds = m_outerBounds;

			ImGui::SetCursorScreenPos(m_innerBounds.min);
			m_requiredArea = { 0, 0 };
			for (const auto& [text, style] : m_text) {
				ImGui::PushFont(g_manager->GetFont(style.fontType, style.fontSize));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(style.color.r, style.color.g, style.color.b, style.color.a));

				auto [width, height] = ImGui::CalcTextSize(text.c_str());
				m_requiredArea.x = std::max(m_requiredArea.x, width);
				m_requiredArea.y += height;

				ImGui::Text(text.c_str());
				ImGui::SameLine();
				ImGui::PopStyleColor();
				ImGui::PopFont();
			}
		}

	private:
		std::vector<std::pair<std::string, Style>> m_text {};
	};
}
