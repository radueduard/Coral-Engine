//
// Created by radue on 10/23/2024.
//

#pragma once

#include <glm/glm.hpp>

#include "object.h"
#include "memory/buffer.h"

namespace mgv {
    class Camera final : public Component
    {
        friend class DebugCamera;
        friend class Mesh;

        inline static std::vector<Camera*> cameras;
        inline static Camera* mainCamera;
    public:
        struct Frustum {
            glm::vec4 left;
            glm::vec4 right;
            glm::vec4 top;
            glm::vec4 bottom;
            glm::vec4 near;
            glm::vec4 far;
        };

        enum Type {
            Perspective = 0,
            Orthographic = 1
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
                float near = 100.0f;
                float far = 0.1f;
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
            glm::mat4 flippedView;
            glm::mat4 flippedInverseView;
        };

        explicit Camera(const Object& object, const CreateInfo &createInfo);
        ~Camera() override = default;

        void Update(double deltaTime) override;
        void LateUpdate(double deltaTime) override;
        void Resize(glm::uvec2 size);

        [[nodiscard]] bool Primary() const { return m_primary; }
        [[nodiscard]] const glm::mat4& Projection() const { return m_projection; }
        [[nodiscard]] const glm::mat4& InverseProjection() const { return m_inverseProjection; }
        [[nodiscard]] const glm::mat4& View() const { return m_view; }
        [[nodiscard]] const glm::mat4& InverseView() const { return m_inverseView; }
        [[nodiscard]] const glm::mat4& FlippedView() const { return m_flippedView; }
        [[nodiscard]] const glm::mat4& FlippedInverseView() const { return m_flippedInverseView; }
        [[nodiscard]] bool Moved() const { return m_moved; }
        [[nodiscard]] glm::uvec2 Resolution() const { return m_viewportSize; }
        // [[nodiscard]] Memory::Buffer* FrustumsBuffer() const { return m_frustumBuffer.get(); }
        [[nodiscard]] Info BufferData() const;

        [[nodiscard]] static Camera* Main() { return mainCamera; }
        [[nodiscard]] static const std::vector<Camera*>& All() { return cameras; }

    private:
        void RecalculateProjection();
        void RecalculateView();

        // void CalculateScreenFrustums(uint32_t chunksOnX, uint32_t chunksOnY);

        glm::mat4 m_projection { 1.0f };
        glm::mat4 m_view { 1.0f };
        glm::mat4 m_inverseProjection { 1.0f };
        glm::mat4 m_inverseView { 1.0f };

        glm::mat4 m_flippedView { 1.0f };
        glm::mat4 m_flippedInverseView { 1.0f };

        Type m_type = Perspective;
        ProjectionData m_projectionData {};
        glm::uvec2 m_viewportSize{0, 0};

        bool m_primary = true;
        bool m_moved = true;
        bool m_changed = false;
        const glm::vec3 m_up = glm::vec3(0, 1, 0);

        // std::unique_ptr<Memory::Buffer> m_frustumBuffer = nullptr;

        void Rotate(float yaw, float pitch);
        void MoveForward(float amount);
        void MoveRight(float amount);
        void MoveUp(float amount);
    };
}
