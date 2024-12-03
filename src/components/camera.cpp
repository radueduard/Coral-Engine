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

#include "core/input.h"

namespace mgv {
    Camera* Camera::mainCamera = nullptr;

    Camera::Camera(const Object& object, const CreateInfo &createInfo)
        : Component(object), m_type(createInfo.type),
          m_projectionData(createInfo.projectionData),
          m_viewportSize(createInfo.size),
          m_primary(createInfo.primary) {

        if (m_primary) {
            if (mainCamera != nullptr) {
                mainCamera->m_primary = false;
            }
            mainCamera = this;
        }

        RecalculateProjection();
        RecalculateView();
    }

    void Camera::Update(const double deltaTime) {
        if (m_primary && Core::Input::IsMouseButtonHeld(MouseButtonRight)) {
            static constexpr float sensitivity = 5.f;
            const auto delta = Core::Input::GetMousePositionDelta();
            Rotate(static_cast<float>(delta.x) * sensitivity, static_cast<float>(delta.y) * sensitivity);

            if (Core::Input::IsKeyHeld(W)) {
                MoveForward(3.0f * static_cast<float>(deltaTime));
            }
            if (Core::Input::IsKeyHeld(S)) {
                MoveForward(-3.0f * static_cast<float>(deltaTime));
            }
            if (Core::Input::IsKeyHeld(A)) {
                MoveRight(-3.0f * static_cast<float>(deltaTime));
            }
            if (Core::Input::IsKeyHeld(D)) {
                MoveRight(3.0f * static_cast<float>(deltaTime));
            }
            if (Core::Input::IsKeyHeld(E)) {
                MoveUp(3.0f * static_cast<float>(deltaTime));
            }
            if (Core::Input::IsKeyHeld(Q)) {
                MoveUp(-3.0f * static_cast<float>(deltaTime));
            }
        }

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

        if (forwardDirection.y > 0) {
            const auto flippedPosition = glm::vec3(position.x, -position.y, position.z);
            m_flippedView = glm::lookAt(flippedPosition, flippedPosition + forwardDirection, m_up);
            m_flippedInverseView = glm::inverse(m_flippedView);
        } else {
            const float distanceToXOZ = - position.y / glm::dot(forwardDirection, m_up);
            const glm::vec3 intersection = position + forwardDirection * distanceToXOZ;
            const auto flippedPosition = glm::vec3(position.x, 2 * intersection.y - position.y, position.z);
            m_flippedView = glm::lookAt(flippedPosition, intersection, m_up);
            m_flippedInverseView = glm::inverse(m_flippedView);
        }

        // Calculate the flipped view from the opposite side of the xOz plane

    }

    void Camera::Rotate(float yaw, float pitch) {
        if (pitch == 0 && yaw == 0)
            return;

        auto forwardDirection = glm::vec3(0, 0, -1);
        const glm::quat rotation = glm::radians(Owner().rotation);
        forwardDirection = rotation * forwardDirection;

        const glm::vec3 right = glm::normalize(glm::cross(forwardDirection, m_up));
        yaw /= static_cast<float>(m_viewportSize.x);
        pitch /= static_cast<float>(m_viewportSize.y);

        const auto rotate = glm::normalize(glm::cross(
            glm::angleAxis(-pitch, right),
            glm::angleAxis(-yaw, m_up)));
        forwardDirection = glm::rotate(rotate, forwardDirection);
        m_moved = true;


        Owner().rotation = glm::degrees(glm::eulerAngles(glm::quatLookAt(forwardDirection, m_up)));
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
