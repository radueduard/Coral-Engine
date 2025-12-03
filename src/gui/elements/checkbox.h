//
// Created by radue on 5/11/2025.
//


#pragma once
#include "element.h"
#include "text.h"

namespace Coral::Reef {
	class Checkbox final : public Element {
	public:
		static Style DefaultStyle() {
			return {
				.size = { 20.f, 20.f },
				// .padding = { 10.f, 10.f, 5.f, 5.f },
				.cornerRadius = 5.f,
				.backgroundColor = Colors::grey[800],
			};
		};

		static Text::Style DefaultTextStyle() {
			return {
				.color = Colors::white,
				.fontSize = 10.f,
				.fontStyle = FontType::Bold,
			};
		};

		explicit Checkbox(
			String name,
			const bool initialValue,
			std::function<void(bool)> function = [](bool){},
			const Style& style = DefaultStyle(),
			const Text::Style& textStyle = DefaultTextStyle()
		) : Element(style), m_name(std::move(name)), m_value(initialValue), m_function(std::move(function)), m_checkMarkStyle(textStyle) {}
		~Checkbox() override = default;

		void Subrender() override {
			const f32 fontSize = std::min(
				m_currentSize.width - m_style.padding.top - m_style.padding.bottom,
				m_checkMarkStyle.fontSize);

			const Math::Vector2f padding = {
				(m_currentSize.width - fontSize) / 2.f,
				(m_currentSize.height - fontSize) / 2.f
			};

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding.x, padding.y));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m_style.cornerRadius);
			ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(m_checkMarkStyle.color));

			f32 fontScale = fontSize / ImGui::GetFontSize();
			ImGui::SetWindowFontScale(fontScale);
			ImGui::SetNextItemWidth(m_currentSize.width);

			if (ImGui::Checkbox(("##" + m_name).c_str(), &m_value)) {
				m_function(m_value);
			}
			ImGui::SetWindowFontScale(1.f);

			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor(1);
		}
	private:
		String m_name;
		bool m_value;
		std::function<void(bool)> m_function;

		Text::Style m_checkMarkStyle;
	};
}
