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

namespace GUI {
    class Image final : public Element {
    public:
        enum Mode {
            Contain,
            Cover,
            Stretch
        };

        explicit Image(const ImTextureID id, const Math::Vector2<float> size = Math::Vector2<float>::Zero(), const float cornerRadius = 0.f, const Mode mode = Contain)
            : m_texture(id), m_cornerRadius(cornerRadius), m_mode(mode) { m_requiredArea = size; }
        ~Image() override = default;

        void Render() override {
            // if (m_requiredArea == Math::Vector2<float>::Zero()) {
            //     m_requiredArea = m_parent->InnerBounds().max - m_parent->InnerBounds().min;
            // }
            m_outerBounds = m_parent->AllocatedArea(this);

            Math::Vector2<float> uv1 = { 0, 0 };
            Math::Vector2<float> uv2 = { 1, 1 };

            if (m_mode == Stretch) {
                m_innerBounds = m_outerBounds;
            } else if (m_mode == Cover) {
                m_innerBounds = m_outerBounds;
                if (m_requiredArea.x > m_requiredArea.y) {
                    const float ratio = m_requiredArea.y / m_requiredArea.x;
                    uv1 = { 0.f, (1.f - ratio) / 2.f };
                    uv2 = { 1.f, (1.f + ratio) / 2.f };
                } else {
                    const float ratio = m_requiredArea.x / m_requiredArea.y;
                    uv1 = { (1.f - ratio) / 2.f, 0.f };
                    uv2 = { (1.f + ratio) / 2.f, 1.f };
                }
            } else if (m_mode == Contain) {
                if (const auto size = m_outerBounds.max - m_outerBounds.min; size.x > size.y) {
                    m_innerBounds.min = { m_outerBounds.min.x + (size.x - size.y) / 2.f, m_outerBounds.min.y };
                    m_innerBounds.max = { m_innerBounds.min.x + size.y, m_outerBounds.max.y };
                } else {
                    m_innerBounds.min = { m_outerBounds.min.x, m_outerBounds.min.y + (size.y - size.x) / 2.f };
                    m_innerBounds.max = { m_outerBounds.max.x, m_innerBounds.min.y + size.x };
                }
            }

            ImGui::SetCursorScreenPos({ m_innerBounds.min.x, m_innerBounds.min.y });
            ImGui::RoundedImage(
                m_texture,
                m_innerBounds.max - m_innerBounds.min, m_cornerRadius,
                uv1, uv2);
        }

    private:
        ImTextureID m_texture;
        float m_cornerRadius;
        Mode m_mode;
    };
}