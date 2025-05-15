//
// Created by radue on 2/10/2025.
//

#pragma once

#include "element.h"
#include "imgui.h"

namespace ImGui {
    static void RoundedImage(const ImTextureID user_texture_id,
        const ImVec2& size, const float diameter,
        const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1),
        const ImVec4& tint_col = ImVec4(1, 1, 1, 1))
    {
        const ImVec2 p_min = GetCursorScreenPos();
        const ImVec2 p_max = {p_min.x + size.x, p_min.y + size.y };
        GetWindowDrawList()->AddImageRounded(
            user_texture_id,
            p_min, p_max,
            uv0, uv1,
            GetColorU32(tint_col), diameter * 0.5f
        );
        Dummy(ImVec2(diameter, diameter));
    }
}

namespace Coral::Reef {
    class Image final : public Element {
    public:
        explicit Image(const ImTextureID id, const Style& style = Style())
            : Element(style), m_texture(id) {}
        ~Image() override = default;

		bool Render() override {
			const bool shouldReset = Element::Render();

            const Math::Vector2 uv1 = { 0.f, 0.f };
            const Math::Vector2 uv2 = { 1.f, 1.f };

            ImGui::SetCursorScreenPos({ m_position.x + m_padding.left, m_position.y + m_padding.top });
            ImGui::RoundedImage(
                m_texture,
                { m_currentSize.width - m_padding.left - m_padding.right, m_currentSize.height - m_padding.top - m_padding.bottom },
                m_cornerRadius,
                ImVec2(uv1), ImVec2(uv2));

			return shouldReset;
        }

        void SetTexture(const ImTextureID id) {
            m_texture = id;
        }

    private:
        ImTextureID m_texture;
    };
}