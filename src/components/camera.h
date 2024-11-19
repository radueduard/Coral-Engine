//
// Created by radue on 10/23/2024.
//

#pragma once

#include <glm/glm.hpp>

#include "object.h"

namespace mgv {
    class Camera final : public Component
    {
    public:
        static Camera* mainCamera;
        enum Type {
            Perspective,
            Orthographic
        };

        union ProjectionData {
            struct Perspective {
                float fov = 45.0f;
                float near = 0.1f;
                float far = 100.0f;
            } perspective;
            struct Orthographic {
                float left = -1.0f;
                float right = 1.0f;
                float top = 1.0f;
                float bottom = -1.0f;
                float near = 0.1f;
                float far = 100.0f;
            } orthographic;

            ProjectionData() : perspective() {}
            explicit ProjectionData(const Perspective &perspective) : perspective(perspective) {}
            explicit ProjectionData(const Orthographic &orthographic) : orthographic(orthographic) {}
        };

        struct CreateInfo {
            bool primary = true;
            Type type = Perspective;
            ProjectionData projectionData;
            glm::uvec2 size = { 800, 600 };
        };

        struct Info {
            glm::mat4 view;
            glm::mat4 projection;
            glm::mat4 inverseView;
            glm::mat4 inverseProjection;
        };

        explicit Camera(const Object& object, const CreateInfo &createInfo);

        void Update(double deltaTime) override;
        void Resize(glm::uvec2 size);

        [[nodiscard]] bool Primary() const { return m_primary; }
        [[nodiscard]] const glm::mat4& Projection() const { return m_projection; }
        [[nodiscard]] const glm::mat4& InverseProjection() const { return m_inverseProjection; }
        [[nodiscard]] const glm::mat4& View() const { return m_view; }
        [[nodiscard]] const glm::mat4& InverseView() const { return m_inverseView; }
        [[nodiscard]] bool Moved() const { return m_moved; }
        [[nodiscard]] Info BufferData() const;

    private:
        void RecalculateProjection();
        void RecalculateView();

        glm::mat4 m_projection { 1.0f };
        glm::mat4 m_view { 1.0f };
        glm::mat4 m_inverseProjection { 1.0f };
        glm::mat4 m_inverseView { 1.0f };

        Type m_type = Perspective;
        ProjectionData m_projectionData {};
        glm::uvec2 m_viewportSize{0, 0};

        bool m_primary = true;
        bool m_moved = true;
        const glm::vec3 m_up = glm::vec3(0, 1, 0);

        void Rotate(float yaw, float pitch);
        void MoveForward(float amount);
        void MoveRight(float amount);
        void MoveUp(float amount);
    public:
        ~Camera() override = default;
    };
}
