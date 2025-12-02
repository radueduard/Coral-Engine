//
// Created by radue on 2/18/2025.
//

#pragma once

#include <utility>
#include <vector>

#include "element.h"
#include "imgui.h"
#include "gui/manager.h"
#include <color/color.h>

namespace Coral::Reef {
	class Text : public Element {
	public:
		enum class Overflow {
			Clip,
			Fade,
			Ellipsis,
		};

		enum class VerticalAlignment
		{
			Top,
			Middle,
			Bottom,
		};

		enum class HorizontalAlignment
		{
			Left,
			Center,
			Right,
		};

		struct Style {
			Color color = Colors::white;
			f32 fontSize;
			FontType fontStyle;
			String fontFamily;
			Overflow overflow;
			u32 maxLines;
			Axis direction;
			VerticalAlignment verticalAlignment = VerticalAlignment::Middle;
			HorizontalAlignment horizontalAlignment = HorizontalAlignment::Left;
		};

		explicit Text(String text, Text::Style textStyle = {}, const Reef::Style& elementStyle = {
			.backgroundColor = Colors::transparent,
		}) : Element(elementStyle, {}), m_text(std::move(text)), m_textStyle(std::move(textStyle)) {}

		~Text() override = default;

	    void Update() override
	    {
	        Element::Update();
	        const auto newSize = CalcSize();
	        if (newSize == m_textSize) {
	            m_shouldResize = false;
	        } else {
	            m_textSize = newSize;
	            m_minSize = m_textSize;
	            m_shouldResize = true;
	        }
	    }

		void Subrender() override {
			Math::Vector2f position;
			if (m_textStyle.horizontalAlignment == HorizontalAlignment::Left) {
				position.x = m_actualRenderedPosition.x;
			} else if (m_textStyle.horizontalAlignment == HorizontalAlignment::Center) {
				position.x = m_actualRenderedPosition.x - (m_textSize.x / 2.f) + (m_currentSize.width / 2.f);
			} else {
				position.x = m_actualRenderedPosition.x - m_textSize.x + m_currentSize.width;
			}

			if (m_textStyle.verticalAlignment == VerticalAlignment::Top) {
				position.y = m_actualRenderedPosition.y;
			} else if (m_textStyle.verticalAlignment == VerticalAlignment::Middle) {
				position.y = m_actualRenderedPosition.y - (m_textSize.y / 2.f) + (m_currentSize.height / 2.f);
			} else {
				position.y = m_actualRenderedPosition.y - m_textSize.y + m_currentSize.height;
			}

			const ImFont* font = Context::GUIManager().GetFont(m_textStyle.fontStyle, m_textStyle.fontSize);
			ImGui::GetWindowDrawList()->AddText(
				font,
				m_textStyle.fontSize,
				{ position.x, position.y },
				ImGui::ColorConvertFloat4ToU32(static_cast<ImVec4>(m_textStyle.color)),
				m_text.c_str(),
				nullptr,
				m_currentSize.width);
		}

	protected:
		[[nodiscard]]
		Math::Vector2<f32> CalcSize() const {
			const auto* font = Context::GUIManager().GetFont(m_textStyle.fontStyle, m_textStyle.fontSize);
			const auto size = font->CalcTextSizeA(
				m_textStyle.fontSize,
				FLT_MAX,
				m_currentSize.width,
				m_text.c_str(),
				nullptr);
			return { size.x, size.y };
		}

		String m_text;
		Text::Style m_textStyle;
		Math::Vector2f m_textSize;
	};

	template<typename... T>
	class DynamicText final : public Text {
	public:
		explicit DynamicText(String format, std::function<T()>... funcs)
			: Text(""), m_format(std::move(format)), m_args(std::make_tuple(std::move(funcs)...))
		{
			m_evaluatedArgs = std::apply([&](const std::function<T()>&... fs) {
				return std::make_tuple(fs()...);
			}, m_args);

			m_text = std::apply(
				[&](const T&... evaluatedArgs) {
					return std::vformat(m_format, std::make_format_args(evaluatedArgs...));
				}, m_evaluatedArgs);

		}

		DynamicText* setTextStyle(const Text::Style& style) {
			m_textStyle = style;
			return this;
		}

		DynamicText* setBoxStyle(const Reef::Style& style) {
			m_style = style;
			return *this;
		}

	    void Update() override
		{
		    auto args = std::apply([&](const std::function<T()>&... fs) {
                return std::make_tuple(fs()...);
            }, m_args);

		    if (args == m_evaluatedArgs) {
		        m_shouldResize = false;
                return;
		    }

		    m_text = std::apply(
                [&](const T&... evaluatedArgs) {
                    return std::vformat(m_format, std::make_format_args(evaluatedArgs...));
                },
                args);

		    m_evaluatedArgs = args;
		    Text::Update();
		}

	private:
		String m_format;
		std::tuple<std::function<T()>...> m_args;
		std::tuple<T...> m_evaluatedArgs;
	};
}
