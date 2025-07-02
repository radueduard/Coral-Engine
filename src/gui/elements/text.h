//
// Created by radue on 2/18/2025.
//

#pragma once

#include <vector>

#include "element.h"
#include "imgui.h"
#include "gui/manager.h"

namespace Coral::Reef {
	class Text final : public Element {
	public:
		struct Style {
			Color color = { 1.f, 1.f, 1.f, 1.f };
			f32 fontSize = 13.f;
			FontType fontType = FontType::Regular;
			Math::Vector2<f32> minSize = { 0.f, 0.f };
		};

		struct Piece {
			String text;
			Style style;

			explicit Piece(String text, Style style = Style())
				: text(std::move(text)), style(std::move(style)) {}
		};

		class Builder {
			friend class Text;
		public:
			explicit Builder(Reef::Style style = Reef::Style()) : m_style(std::move(style)) {
				m_text.emplace_back();
			}

			Builder& Add(String text, Text::Style style = Style()) {
				m_text.back().emplace_back(std::move(text), std::move(style));
				return *this;
			}

			Builder& NewLine() {
				m_text.emplace_back();
				return *this;
			}

			[[nodiscard]]
			Text* Build() {
				return new Text(this);
			}

		private:
			Reef::Style m_style {};
			std::vector<std::vector<Piece>> m_text {};
		};

		explicit Text(Builder* builder) : Element(builder->m_style) {
			m_text = std::move(builder->m_text);
		}

		explicit Text(const Piece& piece, const Reef::Style& style = Reef::Style())
			: Element(style) {
			m_text.emplace_back();
			m_text.back().emplace_back(piece);
		}

		explicit Text(const String& text, const Reef::Style& style = Reef::Style())
			: Element(style) {
			m_text.emplace_back();
			m_text.back().emplace_back(text, Style {});
		}

		~Text() override = default;
		bool Render() override {
			const bool shouldReset = Element::Render();

			const f32 yOffset = (m_currentSize.height - m_textBlockSize.height) / 2.f;
			Math::Vector2<f32> position = m_position + Math::Vector2 { m_padding.left, m_padding.top };
			for (const auto& line : m_text) {
				const f32 lineHeight = std::ranges::fold_left(line, 0.f, [](const f32 acc, const Piece& piece) {
					const auto textSize = GlobalManager().GetFont(piece.style.fontType, piece.style.fontSize)
						->CalcTextSizeA(piece.style.fontSize, FLT_MAX, 0.f, piece.text.c_str(), nullptr, nullptr);
					return std::max(acc, textSize.y);
				});
				for (const auto& [text, style] : line) {
					ImGui::PushFont(GlobalManager().GetFont(style.fontType, style.fontSize));
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(style.color.r, style.color.g, style.color.b, style.color.a));

					const auto textSize = ImGui::CalcTextSize(text.c_str());
					auto usedPosition = position + Math::Vector2 { 0.f, lineHeight - textSize.y } + Math::Vector2 { 0.f, yOffset };
					ImGui::SetCursorScreenPos(ImVec2(usedPosition));
					ImGui::Text("%s", text.c_str());

					position.x += textSize.x + m_spacing;

					ImGui::PopStyleColor();
					ImGui::PopFont();
				}
				position.y += lineHeight + m_spacing;
				position.x = m_position.x + m_padding.left;
			}

			return shouldReset;
		}

		void RecreateRequired() override {
			m_textBlockSize = CalcSize();
			if (m_baseSize.width == Shrink) {
				m_baseSize.width = std::max(m_baseSize.width, m_textBlockSize.width);
			}
			if (m_baseSize.height == Shrink) {
				m_baseSize.height = std::max(m_baseSize.height, m_textBlockSize.height);
			}
			Element::RecreateRequired();
		}
	private:
		Math::Vector2<f32> CalcSize() const {
			Math::Vector2 size = { 0.f, 0.f };
			for (const auto& line : m_text) {
				f32 lineHeight = 0.f;
				for (const auto& piece : line) {
					const auto font = GlobalManager().GetFont(piece.style.fontType, piece.style.fontSize);
					auto textSize = font->CalcTextSizeA(piece.style.fontSize, FLT_MAX, 0.f, piece.text.c_str(), nullptr, nullptr);
					lineHeight = std::max(lineHeight, textSize.y);
					size.x += textSize.x;
				}
				if (!line.empty()) {
					size.x += m_spacing * static_cast<f32>(line.size() - 1);
				}
				size.y += lineHeight;
			}
			if (!m_text.empty()) {
				size.y += m_spacing * static_cast<f32>(m_text.size() - 1);
			}
			size.x += m_padding.left + m_padding.right;
			size.y += m_padding.top + m_padding.bottom;
			return size;
		}

		std::vector<std::vector<Piece>> m_text {};
		Math::Vector2<f32> m_textBlockSize;
	};
}
