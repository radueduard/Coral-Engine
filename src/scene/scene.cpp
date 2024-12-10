//
// Created by radue on 10/24/2024.
//

#include "scene.h"

#include <queue>

#include "imgui.h"
#include "components/camera.h"
#include "graphics/objects/cubeMap.h"
#include "graphics/programs/debugCamera.h"
#include "graphics/programs/skyBox.h"

namespace mgv {
    Scene::Scene() : m_root(std::make_unique<Object>()) {
        auto firstCamera = std::make_unique<Object>("Main Camera");
        auto firstCameraCreateInfo = Camera::CreateInfo {
            .primary = true,
            .type = Camera::Type::Perspective,
            .size = { 800, 600 }
        };
        firstCameraCreateInfo.projectionData.perspective = {
            .fov = 75.0f,
            .near = 0.01f,
            .far = 1000.0f
        };
        firstCamera->Add<Camera>(firstCameraCreateInfo);
        m_root->AddChild(std::move(firstCamera));

        auto secondCamera = std::make_unique<Object>("Second Camera");
        auto secondCameraCreateInfo = Camera::CreateInfo {
            .primary = false,
            .type = Camera::Type::Perspective,
            .size = { 800, 600 }
        };
        secondCameraCreateInfo.projectionData.perspective = {
            .fov = 75.0f,
            .near = 0.01f,
            .far = 1000.0f
        };
        secondCamera->Add<mgv::Camera>(secondCameraCreateInfo);
        m_root->AddChild(std::move(secondCamera));
    }

    void Scene::Init() {
        SkyBox::CreateInfo skyBoxCreateInfo {
            .cubeMap = Graphics::CubeMap::Builder()
                .PositiveX("models/textures/cubeMapNight/pos_x.png")
                .NegativeX("models/textures/cubeMapNight/neg_x.png")
                .PositiveY("models/textures/cubeMapNight/pos_y.png")
                .NegativeY("models/textures/cubeMapNight/neg_y.png")
                .PositiveZ("models/textures/cubeMapNight/pos_z.png")
                .NegativeZ("models/textures/cubeMapNight/neg_z.png")
                .Build()
        };
        m_skyBoxProgram = std::make_unique<SkyBox>(skyBoxCreateInfo);

        m_debugCamera = std::make_unique<DebugCamera>();
    }

    void Scene::Update(const double deltaTime) const {
        std::queue<Object*> queue;
        queue.push(Root().get());
        while (!queue.empty()) {
            const auto object = queue.front();
            queue.pop();

            for (const auto& component : object->Components()) {
                component->Update(deltaTime);
            }

            for (const auto& child : object->Children()) {
                queue.push(child);
            }
        }
    }

    void Scene::LateUpdate(const double deltaTime) const {
        std::queue<Object*> queue;
        queue.push(Root().get());
        while (!queue.empty()) {
            const auto object = queue.front();
            queue.pop();

            for (const auto& component : object->Components()) {
                component->LateUpdate(deltaTime);
            }

            for (const auto& child : object->Children()) {
                queue.push(child);
            }
        }
    }

    bool Scene::NodeRender(Object* object) {
        bool wasChildClicked = false;
        if (ImGui::TreeNode(object->Name().c_str())) {
            for (const auto& child : object->Children()) {
                wasChildClicked |= NodeRender(child);
            }
            ImGui::TreePop();
        }
        if (ImGui::IsItemClicked() && !wasChildClicked) {
            m_selectedObject = object;
        }
        return !wasChildClicked;
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
    }

}
