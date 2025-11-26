//
// Created by radue on 10/23/2024.
//

#pragma once

#include "component.h"
#include "math/matrix.h"
#include "math/vector.h"

namespace Coral::Reef {
    class CameraTemplate;
}

namespace Coral::ECS {
    class Camera final : public Component {
        friend class DebugCamera;
        friend class Reef::CameraTemplate;

    public:
        struct Frustum {
            Math::Vector4<f32> left;
            Math::Vector4<f32> right;
            Math::Vector4<f32> top;
            Math::Vector4<f32> bottom;
            Math::Vector4<f32> near;
            Math::Vector4<f32> far;
        };

        enum class Type {
            Orthographic,
            Perspective,
        };

        struct Perspective {
            f32 fov = 45.0f;
            f32 near = 0.1f;
            f32 far = 100.0f;
        } perspective;

        struct Orthographic {
            f32 left = -1.0f;
            f32 right = 1.0f;
            f32 top = 1.0f;
            f32 bottom = -1.0f;
            f32 near = 0.1f;
            f32 far = 100.0f;
        } orthographic;

        union Projection {
            Perspective perspective;
            Orthographic orthographic;

            Projection() : perspective() {}
            explicit Projection(const Perspective &perspective) : perspective(perspective) {}
            explicit Projection(const Orthographic &orthographic) : orthographic(orthographic) {}
        };

        struct ProjectionData {
            Type type;
            Projection data;

            ProjectionData(): type(Type::Perspective), data(Perspective()) {}
            explicit ProjectionData(const Type type) : type(type) {
                if (type == Type::Perspective) {
                    data.perspective = Perspective();
                } else if (type == Type::Orthographic) {
                    data.orthographic = Orthographic();
                }
            }
            explicit ProjectionData(const Perspective &perspective) : type(Type::Perspective), data(perspective) {}
            explicit ProjectionData(const Orthographic &orthographic) : type(Type::Orthographic), data(orthographic) {}
        };

        struct CreateInfo {
            ProjectionData projectionData = {};
            Math::Vector2u size = { 800u, 600u };
        };

        explicit Camera(const CreateInfo &createInfo);
        ~Camera() override = default;

        void Resize(const Math::Vector2<u32>& size);

        [[nodiscard]] bool Primary() const { return m_primary; }
        bool& Primary() { return m_primary; }

        [[nodiscard]] const Math::Matrix4<f32>& Projection() const { return m_projection; }
        [[nodiscard]] const Math::Matrix4<f32>& InverseProjection() const { return m_inverseProjection; }
        [[nodiscard]] const Math::Matrix4<f32>& View() const { return m_view; }
        [[nodiscard]] const Math::Matrix4<f32>& InverseView() const { return m_inverseView; }
        [[nodiscard]] bool& Moved() { return m_moved; }
		[[nodiscard]] bool& Changed() { return m_changed; }

        [[nodiscard]] Math::Vector2<u32> Resolution() const { return m_viewportSize; }

        [[nodiscard]] float AspectRatio() const { return static_cast<float>(m_viewportSize.x) / static_cast<float>(m_viewportSize.y); }
        [[nodiscard]] ProjectionData& GetProjectionData() { return m_projectionData; }

    	void Move(const Math::Vector3<f32>& amount);
    	void Rotate(f32 yaw, f32 pitch);

        void RecalculateProjection();
        void RecalculateView();
    private:
        Math::Matrix4<f32> m_projection { 1.0f };
        Math::Matrix4<f32> m_view { 1.0f };
        Math::Matrix4<f32> m_inverseProjection { 1.0f };
        Math::Matrix4<f32> m_inverseView { 1.0f };

        ProjectionData m_projectionData {};
        Math::Vector2<u32> m_viewportSize { 0u, 0u };

        bool m_primary = true;
        bool m_moved = true;
        bool m_changed = false;

    	const Math::Vector3<f32> FORWARD = { 0.0f, 0.0f, -1.0f };
    	const Math::Vector3<f32> UP = { 0.0f, -1.0f, 0.0f };
    };
}