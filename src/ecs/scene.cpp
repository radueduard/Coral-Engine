//
// Created by radue on 10/24/2024.
//

#include "scene.h"
#include <queue>
#include "IconsFontAwesome6.h"

#include "components/camera.h"
#include "gui/reef.h"

#include "ecs/entity.h"
#include "gui/templates/inspector.h"

#include <entt/entt.hpp>

namespace Coral::ECS {
    Scene::Scene() {
        m_currentScene = this;

        m_root = std::make_unique<Entity>("Root");

        auto firstCamera = std::make_unique<Entity>("First Camera");
        auto firstCameraCreateInfo = Camera::CreateInfo {
            .projectionData = Camera::ProjectionData(Camera::Type::Perspective),
            .size = { 800, 600 }
        };
        firstCamera->Add<Camera>(firstCameraCreateInfo);
        m_root->Add<Camera>(firstCameraCreateInfo);
        m_root->AddChild(std::move(firstCamera));

        auto secondCamera = std::make_unique<Entity>("Second Camera");
        auto secondCameraCreateInfo = Camera::CreateInfo {
            .projectionData = Camera::ProjectionData(Camera::Type::Orthographic),
            .size = { 800, 600 }
        };
        secondCamera->Add<Camera>(secondCameraCreateInfo);
        m_root->AddChild(std::move(secondCamera));

        m_root->Get<Camera>().Primary() = true;

        m_inspectorTemplate = new Reef::EntityInspector();
    }

    void Scene::OnGUIAttach() {
        AddDockable(
            "Scene View",
            new Reef::Dockable(
                ICON_FA_LIST "   Scene View",
                Reef::Style {
                    .size = { 300.f, 0.f },
                    .padding = { 10.f, 10.f, 10.f, 10.f },
                    .spacing = 10.f,
                    .backgroundColor = { 0.0f, 0.0f, 0.0f, 1.f },
                },
                {
                    new Reef::TreeView<Entity, entt::entity>(
                        *m_root,
                        [this](Entity& object) {
                            m_selectedObject = object.Id();
                            AddDockable(
                                "Object Inspector",
                                new Reef::Dockable(
                                    ICON_FA_INFO "   Object Inspector",
                                    Reef::Style {
                                        .size = { 300.f, Reef::Grow },
                                        .padding = { 10.f, 10.f, 10.f, 10.f },
                                        .spacing = 10.f,
                                        .backgroundColor = { 0.0f, 0.0f, 0.0f, 1.f },
                                    },
                                    {
                                        new Reef::Element(
                                            {},
                                            {
                                                m_inspectorTemplate->Build(object),
                                            }
                                        )
                                    }
                                )
                            );
                        },
                        Reef::Style {
                            .size = { Reef::Grow, Reef::Grow },
                            .padding = { 10.f, 10.f, 10.f, 10.f },
                            .cornerRadius = 10.f,
                            .backgroundColor = { 0.1f, 0.1f, 0.1f, 1.f },
                        }
                    ),
                }
            )
        );

        // m_guiBuilder["Object Inspector"] = [this] () -> GUI::Element* {
        //     if (m_selectedObject == entt::null) {
        //         return new GUI::Dockable(
        //             ICON_FA_INFO "   Object Inspector",
        //             new GUI::Text("No object selected", GUI::Text::Style{ { 0.8f, 0.8f, 0.8f, 1.f }, 20.f, GUI::FontType::Black }),
        //             { 10.f, 10.f }
        //         );
        //     }
        //     return m_inspectorTemplate.Build(*ECS::World::Get().Registry().get<ECS::Entity*>(m_selectedObject));
        // };
    }

    Scene& Scene::Get() {
        try {
            return *m_currentScene;
        } catch (const std::exception& e) {
            throw std::runtime_error("There is no scene active" + std::string(e.what()));
        }
    }

    Camera& Scene::MainCamera() {
        const auto cameras = m_registry.view<Camera>();
        for (auto camera : cameras) {
            if (m_registry.get<Camera>(camera).Primary()) {
                return m_registry.get<Camera>(camera);
            }
        }
        throw std::runtime_error("No primary camera found");
    }
}
