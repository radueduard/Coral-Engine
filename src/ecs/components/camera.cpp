//
// Created by radue on 10/23/2024.
//

#include "camera.h"

#include "ecs/entity.h"
#include "math/constants.h"
#include "math/transform.h"

#include "gui/elements/popup.h"

namespace Coral::ECS {
    Camera::Camera(const CreateInfo &createInfo)
		: m_projectionData(createInfo.projectionData),
          m_viewportSize(createInfo.size),
          m_primary(false)
	{
    	m_cameraBuffer = Memory::Buffer::Builder()
			.InstanceCount(1)
    		.InstanceSize(sizeof(GPU::Camera))
    		.UsageFlags(vk::BufferUsageFlagBits::eUniformBuffer)
    		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
    		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
			.Build();
    }

    void Camera::Resize(const Math::Vector2<u32>& size) {
        if (size == m_viewportSize || size.x == 0 || size.y == 0)
            return;

        m_viewportSize = size;
        m_changed = true;
    }

	void Camera::Update() {
    	bool updateBuffer = false;
		const auto& transform = Entity().Get<Transform>();
	    if (m_changed || transform.Changed()) {
		    RecalculateProjection();
	    	updateBuffer = true;
	    }
	    if (m_moved || transform.Changed()) {
		    RecalculateView();
	    	updateBuffer = true;
	    }

    	if (updateBuffer) {
			auto cameras =m_cameraBuffer->Map<GPU::Camera>();
    		cameras[0] = GPU::Camera {
    			.view = m_view,
				.projection = m_projection,
				.inverseView = m_inverseView,
				.inverseProjection = m_inverseProjection,
			};
    		m_cameraBuffer->Flush();
    		m_cameraBuffer->Unmap();
    	}
    }

	void Camera::Move(const Math::Vector3<f32>& amount) {
    	auto& transform = Entity().Get<Transform>();

		const auto rotation = Math::Quaternion(Math::Radians<f32, 3>(transform.rotation));
		const auto forward = rotation * m_forward;
		const auto up = m_up;
		const auto right = -Math::Vector3<f32>::Cross(forward, up).Normalized();

		transform.position += (forward * amount.z) + (up * amount.y) + (right * amount.x);
		m_moved = true;
	}

	void Camera::Rotate(f32 yaw, f32 pitch) {
		if (yaw == 0.f && pitch == 0.f)
    		return;

    	auto& transform = Entity().Get<Transform>();

		const auto rotation = Math::Quaternion(Math::Radians<f32, 3>(transform.rotation));
    	auto forward = rotation * m_forward;
    	const auto right = -forward.Cross(m_up).Normalized();

    	yaw /= static_cast<f32>(m_viewportSize.x);
    	pitch /= static_cast<f32>(m_viewportSize.y);

    	const auto rotate = Math::Quaternion<>::Cross(
			Math::Quaternion<>::FromAxisAngle(-pitch, right),
			Math::Quaternion<>::FromAxisAngle(-yaw, m_up)).Normalized();
    	forward = Math::Rotate(rotate, forward).Normalized();

    	transform.rotation = Math::Degrees<f32, 3>(Math::Quaternion<>::ToEulerAngles(Math::LookAt(forward, m_up)));

    	m_moved = true;
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
    	m_changed = false;
    }

    void Camera::RecalculateView() {
    	const auto& transform = Entity().Get<Transform>();

        m_view = Math::LookAt(
            transform.position,
            transform.position + Math::Direction(Math::Radians<f32, 3>(transform.rotation)),
            m_up);
        m_inverseView = m_view.Inverse();
    	m_moved = false;
    }
}
