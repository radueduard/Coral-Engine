//
// Created by radue on 10/23/2024.
//

#include "camera.h"

#define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "imgui.h"
#include "core/input.h"
#include "graphics/objects/mesh.h"
#include "memory/buffer.h"

namespace mgv {
    Camera::Camera(const Object& object, const CreateInfo &createInfo)
        : Component(object),
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
        // CalculateScreenFrustums(64, 64);

        cameras.emplace_back(this);
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
        }

        if (m_changed) {
            RecalculateProjection();
            // CalculateScreenFrustums(64, 64);
        }
    }

    void Camera::LateUpdate(double deltaTime) {
        if (m_moved) {
            m_moved = false;
        }

        if (m_changed) {
            m_changed = false;
        }
    }

    void Camera::Resize(const glm::uvec2 size) {
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
            .flippedView = m_flippedView,
            .flippedInverseView = m_flippedInverseView
        };
    }

    void Camera::RecalculateProjection() {
        switch (m_projectionData.type) {
            case Type::Perspective: {
                m_projection = glm::perspectiveFov(
                    glm::radians(m_projectionData.data.perspective.fov),
                    static_cast<float>(m_viewportSize.x),
                    static_cast<float>(m_viewportSize.y),
                    m_projectionData.data.perspective.near,
                    m_projectionData.data.perspective.far);
            }
            break;
            case Type::Orthographic: {
                m_projection = glm::ortho(
                    m_projectionData.data.orthographic.left,
                    m_projectionData.data.orthographic.right,
                    m_projectionData.data.orthographic.bottom,
                    m_projectionData.data.orthographic.top,
                    m_projectionData.data.orthographic.near,
                    m_projectionData.data.orthographic.far);
            }
            break;
            default: {
                throw std::runtime_error("Camera::RecalculateProjection : Invalid projection type");
            }
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
    }

    // void Camera::CalculateScreenFrustums(const uint32_t chunksOnX, const uint32_t chunksOnY) {
    //     if (!m_primary)
    //         return;
    //
    //     if (m_frustumBuffer == nullptr || m_frustumBuffer->InstanceCount() != chunksOnX * chunksOnY) {
    //         m_frustumBuffer = std::make_unique<Memory::Buffer>(
    //             m_device,
    //             sizeof(Frustum), chunksOnX * chunksOnY,
    //             vk::BufferUsageFlagBits::eStorageBuffer,
    //             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    //     }
    //
    //     const float aspectRatio = static_cast<float>(m_viewportSize.x) / static_cast<float>(m_viewportSize.y);
    //     const float fov = m_projectionData.perspective.fov;
    //     const float near = m_projectionData.perspective.near;
    //     const float far = m_projectionData.perspective.far;
    //
    //     const float tanHalfFov = tan(glm::radians(fov) / 2.0f);
    //     const float nearHeight = tanHalfFov * near;
    //     const float nearWidth = nearHeight * aspectRatio;
    //
    //     const auto nearTopLeft = glm::vec3(-nearWidth, nearHeight, -near);
    //     glm::vec2 deltaNear = 2.0f * glm::vec2(nearWidth, -nearHeight) / glm::vec2(chunksOnX, chunksOnY);
    //
    //     auto frustums = m_frustumBuffer->Map<Frustum>();
    //     for (uint32_t i = 0; i < chunksOnX; i++) {
    //         for (uint32_t j = 0; j < chunksOnY; j++) {
    //             auto chunk = glm::vec2(static_cast<float>(i), static_cast<float>(j));
    //
    //             glm::vec3 topLeft = glm::vec3(nearTopLeft.x + deltaNear.x * chunk.x, nearTopLeft.y + deltaNear.y * chunk.y, -near);
    //             glm::vec3 topRight = glm::vec3(nearTopLeft.x + deltaNear.x * (chunk.x + 1), nearTopLeft.y + deltaNear.y * chunk.y, -near);
    //             glm::vec3 bottomLeft = glm::vec3(nearTopLeft.x + deltaNear.x * chunk.x, nearTopLeft.y + deltaNear.y * (chunk.y + 1), -near);
    //             glm::vec3 bottomRight = glm::vec3(nearTopLeft.x + deltaNear.x * (chunk.x + 1), nearTopLeft.y + deltaNear.y * (chunk.y + 1), -near);
    //
    //             glm::vec3 center = ((topLeft + topRight) / 2.0f + (bottomLeft + bottomRight) / 2.0f) / 2.0f;
    //
    //             glm::vec3 dirTopLeft = glm::normalize(topLeft);
    //             glm::vec3 dirTopRight = glm::normalize(topRight);
    //             glm::vec3 dirBottomLeft = glm::normalize(bottomLeft);
    //             glm::vec3 dirBottomRight = glm::normalize(bottomRight);
    //
    //             glm::vec3 rightNormal = glm::normalize(glm::cross(dirTopRight, dirBottomRight));
    //             glm::vec3 leftNormal = glm::normalize(glm::cross(dirBottomLeft, dirTopLeft));
    //             glm::vec3 topNormal = glm::normalize(glm::cross(dirTopLeft, dirTopRight));
    //             glm::vec3 bottomNormal = glm::normalize(glm::cross(dirBottomRight, dirBottomLeft));
    //
    //             frustums[i * chunksOnY + j] = {
    //                 .left = glm::vec4(leftNormal * glm::sign(glm::dot(leftNormal, center)), 0.0f),
    //                 .right = glm::vec4(rightNormal * glm::sign(glm::dot(rightNormal, center)), 0.0f),
    //                 .top = glm::vec4(topNormal * glm::sign(glm::dot(topNormal, center)), 0.0f),
    //                 .bottom = glm::vec4(bottomNormal * glm::sign(glm::dot(bottomNormal, center)), 0.0f),
    //                 .near = glm::vec4(0.0f, 0.0f, -1.0f, near),
    //                 .far = glm::vec4(0.0f, 0.0f, 1.0f, far)
    //             };
    //         }
    //     }
    //     m_frustumBuffer->Flush();
    //     m_frustumBuffer->Unmap();
    // }

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
