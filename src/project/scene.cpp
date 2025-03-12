//
// Created by radue on 10/24/2024.
//

#include "scene.h"

#include <queue>

#include "IconsFontAwesome6.h"

#include "components/camera.h"
#include "gui/elements/dockable.h"
#include "gui/elements/treeView.h"

namespace mgv {
    Scene::Scene(): m_root(std::make_unique<Object>()) {
        auto firstCamera = std::make_unique<Object>("Main Camera");
        auto firstCameraCreateInfo = Camera::CreateInfo {
            .primary = true,
            .projectionData = {},
            .size = { 800, 600 }
        };
        firstCamera->Add<Camera>(firstCameraCreateInfo);
        m_root->AddChild(std::move(firstCamera));

        auto secondCamera = std::make_unique<Object>("Second Camera");
        auto secondCameraCreateInfo = Camera::CreateInfo {
            .primary = false,
            .projectionData = {},
            .size = { 800, 600 }
        };
        secondCamera->Add<mgv::Camera>(secondCameraCreateInfo);
        m_root->AddChild(std::move(secondCamera));

        m_testTexture = Texture::FromFile("assets/textures/brick-wall/brick-wall_albedo.png");
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

    void Scene::OnGUIAttach() {
        m_guiBuilder["Scene View"] = [this] {
            return new GUI::Dockable(
                ICON_FA_LIST "   Scene View",
                new GUI::TreeView<Object>(
                    Root().get(),
                    [this] (Object* object) {
                        if (object != m_selectedObject) {
                            ResetElement("Object Inspector");
                            m_selectedObject = object;
                        }
                    }
                ),
                10.f
            );
        };

        m_guiBuilder["Object Inspector"] = [this] {
            return m_inspectorTemplate.Build(m_selectedObject);
        };
    }
}
