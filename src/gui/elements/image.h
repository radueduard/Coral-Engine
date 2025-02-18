//
// Created by radue on 2/10/2025.
//

#pragma once

#include "element.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"

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
    class Image : public Element {
    public:
        enum Mode {
            Contain,
            Cover,
            Stretch
        };


        Image(const vk::ImageView imageView, const vk::Sampler sampler, vk::ImageLayout imageLayout, const float cornerRadius = 0.f, const Mode mode = Contain)
            : m_texture(ImGui_ImplVulkan_AddTexture(sampler, imageView, static_cast<VkImageLayout>(imageLayout))), m_cornerRadius(cornerRadius), m_mode(mode) {
            m_allocatedArea = { 512, 512 };
        }
        ~Image() override {
            ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(m_texture));
        }

        void Render() override {
            m_startPoint = m_parent->StartPoint(this);
            m_availableArea = m_parent->AllocatedArea(this);

            glm::vec2 size = { 0, 0 };
            glm::vec2 uv1 = { 0, 0 };
            glm::vec2 uv2 = { 1, 1 };

            if (m_mode == Stretch) {
                size = m_availableArea;
            } else if (m_mode == Cover) {
                size = m_availableArea;
                if (size.x > size.y) {
                    const float ratio = size.y / size.x;
                    uv1 = { 0.f, (1.f - ratio) / 2.f };
                    uv2 = { 1.f, (1.f + ratio) / 2.f };
                } else {
                    const float ratio = size.x / size.y;
                    uv1 = { (1.f - ratio) / 2.f, 0.f };
                    uv2 = { (1.f + ratio) / 2.f, 1.f };
                }
            } else if (m_mode == Contain) {
                size = m_allocatedArea;
            }

            ImGui::SetCursorPos({ m_startPoint.x, m_startPoint.y });
            ImGui::RoundedImage(
                m_texture,
                ImVec2(size.x, size.y), m_cornerRadius,
                ImVec2(uv1.x, uv1.y), ImVec2(uv2.x, uv2.y));
        }

        glm::vec2 StartPoint(Element *element) override {
            return { 0, 0 };
        }

        glm::vec2 AllocatedArea(Element *element) override {
            return { 0, 0 };
        }

    private:
        ImTextureID m_texture;
        float m_cornerRadius;
        Mode m_mode;
    };
}