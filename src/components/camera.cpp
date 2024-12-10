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
    Camera::Frustum::Frustum(float fov, float aspectRatio, float near, float far) {
        float tanHalfFov = tan(glm::radians(fov) / 2.0f);
        float nearHeight = tanHalfFov * near;
        float nearWidth = nearHeight * aspectRatio;
        float farHeight = tanHalfFov * far;
        float farWidth = farHeight * aspectRatio;

        glm::vec3 nearCenter = glm::vec3(0.0f, 0.0f, near);
        glm::vec3 farCenter = glm::vec3(0.0f, 0.0f, far);

        glm::vec3 nearTopLeft = nearCenter + glm::vec3(-nearWidth, nearHeight, 0.0f);
        glm::vec3 nearTopRight = nearCenter + glm::vec3(nearWidth, nearHeight, 0.0f);
        glm::vec3 nearBottomLeft = nearCenter + glm::vec3(-nearWidth, -nearHeight, 0.0f);
        glm::vec3 nearBottomRight = nearCenter + glm::vec3(nearWidth, -nearHeight, 0.0f);

        glm::vec3 farTopLeft = farCenter + glm::vec3(-farWidth, farHeight, 0.0f);
        glm::vec3 farTopRight = farCenter + glm::vec3(farWidth, farHeight, 0.0f);
        glm::vec3 farBottomLeft = farCenter + glm::vec3(-farWidth, -farHeight, 0.0f);
        glm::vec3 farBottomRight = farCenter + glm::vec3(farWidth, -farHeight, 0.0f);

        glm::vec3 topLeft = glm::normalize(farTopLeft - nearTopLeft);
        glm::vec3 topRight = glm::normalize(farTopRight - nearTopRight);
        glm::vec3 bottomLeft = glm::normalize(farBottomLeft - nearBottomLeft);
        glm::vec3 bottomRight = glm::normalize(farBottomRight - nearBottomRight);

        glm::vec3 left = glm::cross(topLeft, bottomLeft);
        glm::vec3 right = glm::cross(bottomRight, topRight);
        glm::vec3 top = glm::cross(topRight, topLeft);
        glm::vec3 bottom = glm::cross(bottomLeft, bottomRight);

        this->left = glm::vec4(left, 0.0f);
        this->right = glm::vec4(right, 0.0f);
        this->top = glm::vec4(top, 0.0f);
        this->bottom = glm::vec4(bottom, 0.0f);
    }

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

        if (m_changed) {
            RecalculateProjection();
            m_changed = false;
        }
    }

    void Camera::Resize(const glm::uvec2 size) {
        if (size == m_viewportSize || size.x == 0 || size.y == 0)
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
            .flippedView = m_flippedView,
            .flippedInverseView = m_flippedInverseView
        };
    }

    void Camera::OnUIRender() {
        Component::OnUIRender();

        ImGui::Text("Projection details");
        if (const char* items[] = { "Perspective", "Orthographic" }; ImGui::Combo("Type", reinterpret_cast<int *>(&m_type), items, IM_ARRAYSIZE(items))) {
            switch (m_type) {
                case Perspective:
                    m_projectionData.perspective = { 75.0f, 0.01f, 100.0f };
                break;
                case Orthographic:
                    m_projectionData.orthographic = {
                        -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f
                    };
                break;
            }
            m_changed = true;
        }

        switch (this->m_type) {
            case Perspective:
                m_changed |= ImGui::DragFloat("Fov", &m_projectionData.perspective.fov, 0.1f, 1.0f, 179.0f);
                m_changed |= ImGui::DragFloat("Near", &m_projectionData.perspective.near, 0.1f, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
                m_changed |= ImGui::DragFloat("Far", &m_projectionData.perspective.far, 100.f, 10.0f, 10000.0f, "%.0f", ImGuiSliderFlags_Logarithmic);
                break;
            case Orthographic:
                m_changed |= ImGui::DragFloat("Left", &m_projectionData.orthographic.left, 0.1f, - 10.0f, 0.0f);
                m_changed |= ImGui::DragFloat("Right", &m_projectionData.orthographic.right, 0.1f, 0.0f, 10.0f);
                m_changed |= ImGui::DragFloat("Bottom", &m_projectionData.orthographic.bottom, 0.1f, -10.0f, 0.0f);
                m_changed |= ImGui::DragFloat("Top", &m_projectionData.orthographic.top, 0.1f, 0.0f, 10.0f);
                m_changed |= ImGui::DragFloat("Near", &m_projectionData.orthographic.near, 0.1f, -10.0f, 0.0f);
                m_changed |= ImGui::DragFloat("Far", &m_projectionData.orthographic.far, 0.1f, 0.0f, 10.0f);
                break;
        }

        ImGui::Text("View details");
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", Owner().position.x, Owner().position.y, Owner().position.z);
        ImGui::Text("Forward: (%.2f, %.2f, %.2f)", m_view[0][2], m_view[1][2], m_view[2][2]);
        ImGui::Text("Up: (%.2f, %.2f, %.2f)", m_view[0][1], m_view[1][1], m_view[2][1]);
        ImGui::Text("Right: (%.2f, %.2f, %.2f)", m_view[0][0], m_view[1][0], m_view[2][0]);
    }

    void Camera::RecalculateProjection() {
        switch (m_type) {
            case Perspective: {
                m_projection = glm::perspectiveFov(
                    glm::radians(m_projectionData.perspective.fov),
                    static_cast<float>(m_viewportSize.x),
                    static_cast<float>(m_viewportSize.y),
                    m_projectionData.perspective.near,
                    m_projectionData.perspective.far);

                const float aspect = static_cast<float>(m_viewportSize.x) / static_cast<float>(m_viewportSize.y);
                m_mesh = Mesh::Frustum(m_projectionData.perspective.fov, aspect, m_projectionData.perspective.near, m_projectionData.perspective.far);
            }
            break;
            case Orthographic:
                m_projection = glm::ortho(
                    m_projectionData.orthographic.left,
                    m_projectionData.orthographic.right,
                    m_projectionData.orthographic.bottom,
                    m_projectionData.orthographic.top,
                    m_projectionData.orthographic.near,
                    m_projectionData.orthographic.far);

                m_mesh = Mesh::Cuboid(m_projectionData.orthographic.left, m_projectionData.orthographic.right,
                                      m_projectionData.orthographic.bottom, m_projectionData.orthographic.top,
                                      m_projectionData.orthographic.near, m_projectionData.orthographic.far);
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
    }

    void Camera::CalculateScreenFrustums(const uint32_t chunksOnX, const uint32_t chunksOnY) {
        if (m_frustumBuffer == nullptr || m_frustumBuffer->InstanceCount() != chunksOnX * chunksOnY) {
            m_frustumBuffer = std::make_unique<Memory::Buffer>(
                sizeof(Frustum), chunksOnX * chunksOnY,
                vk::BufferUsageFlagBits::eStorageBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            // m_frustumBuffer->Map<Frustum>();
            // for (uint32_t i = 0; i < chunksOnX * chunksOnY; i++) {
            //     Frustum frustum = {
            //
            //     };
            //     m_frustumBuffer->WriteAt(i, frustum);
            // }
        }


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
