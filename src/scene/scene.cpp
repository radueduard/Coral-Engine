//
// Created by radue on 10/24/2024.
//

#include "scene.h"

#include <queue>

#include "imgui.h"
#include "components/camera.h"

namespace mgv {
    Scene::Scene() : m_root(std::make_unique<Object>()), m_camera(std::make_unique<Object>()) {
        auto cameraCreateInfo = Camera::CreateInfo {
            .primary = true,
            .type = Camera::Type::Perspective,
            .size = { 800, 600 }
        };
        cameraCreateInfo.projectionData.perspective = {
            .fov = 75.0f,
            .near = 0.01f,
            .far = 1000.0f
        };
        m_camera->Add<mgv::Camera>(cameraCreateInfo);

        auto secondCamera = std::make_unique<Object>("Second Camera");
        cameraCreateInfo.primary = false;
        cameraCreateInfo.size = { 800, 800 };
        cameraCreateInfo.projectionData.perspective.fov = 45.0f;
        cameraCreateInfo.projectionData.perspective.near = 0.1f;
        cameraCreateInfo.projectionData.perspective.far = 100.0f;
        secondCamera->Add<mgv::Camera>(cameraCreateInfo);

        m_root->AddChild(std::move(secondCamera));
    }

    void Scene::Update(const double deltaTime) const {
        m_camera->Get<mgv::Camera>().value()->Update(deltaTime);

    }

    void Scene::NodeRender(Object* object) {
        if (ImGui::TreeNode(object->Name().c_str())) {
            for (const auto& child : object->Children()) {
                NodeRender(child);
            }
            ImGui::TreePop();
        }
        if (ImGui::IsItemClicked()) {
            m_selectedObject = object;
        }
    }

    void Scene::OnUIRender() {
        ImGui::Begin("Scene Tree");
        NodeRender(m_root.get());
        ImGui::End();

        if (m_selectedObject != nullptr) {
            ImGui::Begin("Selected Object");
            ImGui::Text("Name: %s", m_selectedObject->Name().c_str());
            ImGui::DragFloat3("Position", &m_selectedObject->position.x, 0.1f);
            ImGui::DragFloat3("Rotation", &m_selectedObject->rotation.x, 0.1f);
            ImGui::DragFloat3("Scale", &m_selectedObject->scale.x, 0.1f);

            for (const auto& component : m_selectedObject->Components()) {
                component->OnUIRender();
            }

            ImGui::End();
        }

        ImGui::Begin("Camera");
        ImGui::DragFloat3("Position", &m_camera->position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &m_camera->rotation.x, 0.1f);
        ImGui::DragFloat3("Scale", &m_camera->scale.x, 0.1f);

        ImGui::Separator();
        m_camera->Get<mgv::Camera>().value()->OnUIRender();
        ImGui::End();
    }

}
