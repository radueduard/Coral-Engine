//
// Created by radue on 10/23/2024.
//

#include "camera.h"

#include "core/scheduler.h"
#include "math/constants.h"
#include "math/transform.h"
#include "memory/descriptor/set.h"

namespace Coral::ECS {
    Camera::Camera(const CreateInfo &createInfo)
        : m_projectionData(createInfo.projectionData),
          m_viewportSize(createInfo.size),
          m_primary(false) {

        RecalculateProjection();
        RecalculateView();
        // CalculateScreenFrustums(64, 64);

        m_setLayout = Memory::Descriptor::SetLayout::Builder()
            .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
            .Build();

        m_buffer = Memory::Buffer::Builder()
    		.InstanceSize(sizeof(Info))
            .InstanceCount(1)
            .MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
    		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
            .UsageFlags(vk::BufferUsageFlagBits::eUniformBuffer)
            .Build();
    }

    void Camera::Resize(const Math::Vector2<u32> size) {
        if (size == m_viewportSize || size.x == 0 || size.y == 0)
            return;

        m_viewportSize = size;
        RecalculateProjection();
        // CalculateScreenFrustums(64, 64);
        m_moved = true;
    }

    Camera::Info Camera::BufferData() const {
        return {
            .view = m_view,
            .projection = m_projection,
            .inverseView = m_inverseView,
            .inverseProjection = m_inverseProjection,
        };
    }

    const Memory::Descriptor::Set& Camera::DescriptorSet() {
        auto span = m_buffer->Map<Info>();
        span[0] = BufferData();
        m_buffer->Flush();
        m_buffer->Unmap();

        if (m_descriptorSet == nullptr) {
            const auto bufferInfo = vk::DescriptorBufferInfo()
                .setBuffer(**m_buffer)
                .setOffset(0)
                .setRange(sizeof(Info));
            m_descriptorSet = Memory::Descriptor::Set::Builder(Core::GlobalScheduler().DescriptorPool(), *m_setLayout)
                .WriteBuffer(0, bufferInfo)
                .Build();
        }
        return *m_descriptorSet;
    }

    void Camera::RecalculateProjection() {
        switch (m_projectionData.type) {
            case Type::Perspective: {
                m_projection = Math::Perspective(
                    Math::Radians<f32>(m_projectionData.data.perspective.fov),
                    { static_cast<f32>(m_viewportSize.x), static_cast<f32>(m_viewportSize.y) },
                    { m_projectionData.data.perspective.near, m_projectionData.data.perspective.far });
            }
            break;
            case Type::Orthographic: {
                m_projection = Math::Orthographic(
                    { m_projectionData.data.orthographic.left, m_projectionData.data.orthographic.right },
                    { m_projectionData.data.orthographic.bottom, m_projectionData.data.orthographic.top },
                    { m_projectionData.data.orthographic.near, m_projectionData.data.orthographic.far });
            }
            break;
            default: {
                throw std::runtime_error("Camera::RecalculateProjection : Invalid projection type");
            }
        }
        m_inverseProjection = m_projection.Inverse();
    }

    void Camera::RecalculateView() {
        m_view = Math::LookAt(
            m_position,
            m_position + m_forward,
            { 0.f, 1.f, 0.f });
        m_inverseView = m_view.Inverse();
    }
}
