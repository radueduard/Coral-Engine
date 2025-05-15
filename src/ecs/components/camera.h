//
// Created by radue on 10/23/2024.
//

#pragma once

#include "math/vector.h"
#include "math/matrix.h"

#include "memory/buffer.h"
#include "memory/descriptor/set.h"

namespace Coral::Reef {
    class CameraTemplate;
}

namespace Coral::ECS {
    class Camera {
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
            Perspective,
            Orthographic,
            Count
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
            ProjectionData projectionData;
            Math::Vector2<u32> size = { 800, 600 };
        };

        struct Info {
            Math::Matrix4<f32> view;
            Math::Matrix4<f32> projection;
            Math::Matrix4<f32> inverseView;
            Math::Matrix4<f32> inverseProjection;
        };

        explicit Camera(const CreateInfo &createInfo);
        ~Camera() = default;

        void Resize(Math::Vector2<u32> size);

        [[nodiscard]] bool Primary() const { return m_primary; }
        bool& Primary() { return m_primary; }

        [[nodiscard]] const Math::Matrix4<f32>& Projection() const { return m_projection; }
        [[nodiscard]] const Math::Matrix4<f32>& InverseProjection() const { return m_inverseProjection; }
        [[nodiscard]] const Math::Matrix4<f32>& View() const { return m_view; }
        [[nodiscard]] const Math::Matrix4<f32>& InverseView() const { return m_inverseView; }
        [[nodiscard]] bool Moved() const { return m_moved; }
        [[nodiscard]] Math::Vector2<u32> Resolution() const { return m_viewportSize; }
        [[nodiscard]] Info BufferData() const;
        [[nodiscard]] const Memory::Descriptor::Set& DescriptorSet();
        [[nodiscard]] const Memory::Descriptor::SetLayout& DescriptorSetLayout() const { return *m_setLayout; }

        [[nodiscard]] float AspectRatio() const { return static_cast<float>(m_viewportSize.x) / static_cast<float>(m_viewportSize.y); }
        [[nodiscard]] ProjectionData& GetProjectionData() { return m_projectionData; }

    private:
        void RecalculateProjection();
        void RecalculateView();

        Math::Matrix4<f32> m_projection { 1.0f };
        Math::Matrix4<f32> m_view { 1.0f };
        Math::Matrix4<f32> m_inverseProjection { 1.0f };
        Math::Matrix4<f32> m_inverseView { 1.0f };

        ProjectionData m_projectionData {};
        Math::Vector2<u32> m_viewportSize { 0, 0 };

        bool m_primary = true;
        bool m_moved = true;
        bool m_changed = false;

        Math::Vector3<f32> m_position { 0.0f, 0.0f, 0.0f };
        Math::Vector3<f32> m_forward { 0.0f, 0.0f, -1.0f };

        std::unique_ptr<Memory::Buffer> m_buffer = nullptr;
        std::unique_ptr<Memory::Descriptor::SetLayout> m_setLayout = nullptr;
        std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet = nullptr;

    };
}