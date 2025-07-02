//
// Created by radue on 10/23/2024.
//

#include "camera.h"

#include "ecs/entity.h"
#include "math/constants.h"
#include "math/transform.h"

#include "gui/elements/popup.h"

namespace Coral::ECS {
    Camera::Camera(const CreateInfo &createInfo) :
          m_projectionData(createInfo.projectionData),
          m_viewportSize(createInfo.size),
          m_primary(false) {
    }

    void Camera::Resize(const Math::Vector2<u32>& size) {
        if (size == m_viewportSize || size.x == 0 || size.y == 0)
            return;

        m_viewportSize = size;
        m_changed = true;
    }

	void Camera::Move(const Math::Vector3<f32>& amount) {
    	const auto& entity = SceneManager::Get().Registry().get<class Entity*>(Entity());
    	auto& transform = entity->Get<Transform>();

		const auto rotation = Math::Quaternion(Math::Radians(transform.rotation));
		const auto forward = rotation * FORWARD;
		const auto up = UP;
		const auto right = -Math::Vector3<f32>::Cross(forward, up).Normalized();

		transform.position += (forward * amount.z) + (up * amount.y) + (right * amount.x);
		m_moved = true;
	}

	void Camera::Rotate(f32 yaw, f32 pitch) {
		if (yaw == 0.f && pitch == 0.f)
    		return;

    	const auto& entity = SceneManager::Get().Registry().get<class Entity*>(Entity());
    	auto& transform = entity->Get<Transform>();

		const auto rotation = Math::Quaternion(Math::Radians(transform.rotation));
    	auto forward = rotation * FORWARD;
    	const auto right = -forward.Cross(UP).Normalized();

    	yaw /= static_cast<f32>(m_viewportSize.x);
    	pitch /= static_cast<f32>(m_viewportSize.y);

    	const auto rotate = Math::Quaternion::Cross(
			Math::Quaternion::FromAxisAngle(-pitch, right),
			Math::Quaternion::FromAxisAngle(-yaw, UP)).Normalized();
    	forward = Math::Rotate(rotate, forward).Normalized();

    	transform.rotation = Math::Degrees(Math::Quaternion::ToEulerAngles(Math::LookAt(forward, UP)));

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
    	const auto& entity = SceneManager::Get().Registry().get<class Entity*>(Entity());
    	const auto& transform = entity->Get<Transform>();

        m_view = Math::LookAt(
            transform.position,
            transform.position + Math::Quaternion(Math::Radians(transform.rotation)) * FORWARD,
            UP);
        m_inverseView = m_view.Inverse();
    	m_moved = false;
    }
}
