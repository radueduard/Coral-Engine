//
// Created by radue on 10/24/2024.
//

#include "scene.h"

#include <queue>

#include "IconsFontAwesome6.h"

#include "components/camera.h"
#include "graphics/objects/cubeMap.h"
#include "gui/elements/button.h"
#include "gui/elements/center.h"
#include "gui/elements/column.h"
#include "gui/elements/dockable.h"
#include "gui/elements/image.h"
#include "gui/elements/row.h"
#include "gui/elements/treeView.h"

namespace mgv {
    Scene::Scene(const Core::Device& device) : m_device(device), m_root(std::make_unique<Object>()) {
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

        m_testTexture = Texture::FromFile(m_device, "assets/textures/brick-wall/brick-wall_albedo.png");
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
        // m_guiObject = std::make_unique<GUI::Dockable>("Scene View",
        //     new GUI::Column("Vertical Layout", std::vector<GUI::Element*>
        //         {
        //             new GUI::Center(
        //                 new GUI::Row("Action Buttons", std::vector<GUI::Element*>
        //                     {
        //                         new GUI::IconButton("Run", ICON_FA_PLAY,
        //                             [] {
        //                                 std::cout << "Run button clicked" << std::endl;
        //                             },
        //                             glm::uvec2(30, 30)
        //                         ),
        //                         new GUI::IconButton("Pause", ICON_FA_PAUSE,
        //                             [] {
        //                                 std::cout << "Pause button clicked" << std::endl;
        //                             },
        //                             glm::uvec2(30, 30)
        //                         ),
        //                         new GUI::IconButton("Stop", ICON_FA_STOP,
        //                             [] {
        //                                 std::cout << "Stop button clicked" << std::endl;
        //                             },
        //                             glm::uvec2(30, 30)
        //                         ),
        //                     },
        //                     10.0f
        //                 ),
        //             true, false),
        //             new GUI::Expanded(
        //                 new GUI::Image(
        //                     m_testTexture->ImageView().Handle(),
        //                     m_testTexture->Sampler().Handle(),
        //                     vk::ImageLayout::eShaderReadOnlyOptimal, 10, GUI::Image::Cover
        //                 )
        //             ),
        //         },
        //         10.0f
        //     ),
        //     glm::vec2(10, 10)
        // );

        m_inspectorPanel = GUI::MakeContainer<GUI::InspectorPanel>();

        m_guiObject = std::make_unique<GUI::Dockable>(
            "Scene tree",
            new GUI::Column("Vertical Layout", std::vector<GUI::Element*>
                {
                    new GUI::Text("Scene tree",
                        GUI::Text::Style {
                            .color = glm::vec4 { 0.5f, 0.5f, 0.5f, 1.f },
                            .fontSize = 20.f,
                            .fontType = GUI::FontType::Black
                        }
                    ),
                    new GUI::Separator(),
                    new GUI::Expanded(
                        new GUI::TreeView(Root().get())
                    ),
                },
                10.f
            ),
            glm::vec2(10, 10)
        );
    }
}
