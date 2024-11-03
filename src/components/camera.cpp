//
// Created by radue on 10/23/2024.
//

#include "camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "memory/descriptor/pool.h"
#include "memory/descriptor/set.h"

namespace mgv {
    Camera* Camera::mainCamera = nullptr;

    Camera::Camera(const Object& object, const CreateInfo &createInfo)
        : Component(object), m_type(createInfo.type),
          m_projectionData(createInfo.projectionData),
          m_viewportSize(createInfo.size),
          m_primary(createInfo.primary) {
        RecalculateProjection();
        RecalculateView();
    }

    void Camera::Update(double deltaTime) {
        if (m_moved) {
            RecalculateView();
            m_moved = false;
        }
    }

    void Camera::Resize(const glm::uvec2 size) {
        if (size == m_viewportSize)
            return;

        m_viewportSize = size;
        RecalculateProjection();
    }

    Camera::Info Camera::BufferData() const {
        return {
            .view = m_view,
            .projection = m_projection,
            .inverseView = m_inverseView,
            .inverseProjection = m_inverseProjection,
        };
    }

    void Camera::RecalculateProjection() {
        switch (m_type) {
            case Perspective:
                m_projection = glm::perspectiveFov(
                    glm::radians(m_projectionData.perspective.fov),
                    static_cast<float>(m_viewportSize.x),
                    static_cast<float>(m_viewportSize.y),
                    m_projectionData.perspective.near,
                    m_projectionData.perspective.far);
                break;
            case Orthographic:
                m_projection = glm::ortho(
                    m_projectionData.orthographic.left,
                    m_projectionData.orthographic.right,
                    m_projectionData.orthographic.bottom,
                    m_projectionData.orthographic.top,
                    m_projectionData.orthographic.near,
                    m_projectionData.orthographic.far);
                break;
        }
        m_inverseProjection = glm::inverse(m_projection);
    }

    void Camera::RecalculateView() {
        const glm::vec3 position = Owner().position;
        const auto rotation = glm::quat(glm::radians(Owner().rotation));
        const glm::vec3 forwardDirection = rotation * glm::vec3(0, 0, -1);

        m_view = glm::lookAt(position, position + forwardDirection, m_up);
        m_inverseView = glm::inverse(m_view);
    }

    void Camera::Rotate(float yaw, float pitch) {
        if (pitch == 0 && yaw == 0)
            return;

        const auto rotation = glm::quat(glm::radians(Owner().rotation));
        glm::vec3 forwardDirection = rotation * glm::vec3(0, 0, -1);

        const glm::vec3 right = glm::normalize(glm::cross(forwardDirection, m_up));
        yaw /= static_cast<float>(m_viewportSize.x);
        pitch /= static_cast<float>(m_viewportSize.y);

        const auto rotate = glm::normalize(glm::cross(
            glm::angleAxis(-pitch, right),
            glm::angleAxis(yaw, m_up)));
        forwardDirection = glm::rotate(rotate, forwardDirection);
        m_moved = true;

        Owner().rotation = glm::eulerAngles(glm::quatLookAt(forwardDirection, m_up));
    }

    void Camera::MoveForward(const float amount) {
        auto forwardDirection = glm::vec3(0, 0, -1);
        const auto rotation = glm::quat(glm::radians(Owner().rotation));
        forwardDirection = rotation * forwardDirection;

        Owner().position += forwardDirection * amount;
        m_moved = true;
    }

    void Camera::MoveRight(const float amount) {
        auto forwardDirection = glm::vec3(0, 0, -1);
        const auto rotation = glm::quat(glm::radians(Owner().rotation));
        forwardDirection = rotation * forwardDirection;

        const auto right = glm::normalize(glm::cross(forwardDirection, m_up));

        Owner().position += right * amount;
        m_moved = true;
    }

    void Camera::MoveUp(const float amount) {
        Owner().position += m_up * amount;
        m_moved = true;
    }


}
