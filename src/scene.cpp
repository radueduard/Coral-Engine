//
// Created by radue on 10/24/2024.
//

#include "scene.h"

namespace mgv {
    Scene::Scene(): m_root(std::make_unique<Object>()), m_camera(std::make_unique<Object>()) {
        auto cameraCreateInfo = Camera::CreateInfo {
            .primary = true,
            .type = Camera::Type::Perspective,
            .size = { 800, 600 }
        };
        cameraCreateInfo.projectionData.perspective = {
            .fov = 45.0f,
            .near = 0.1f,
            .far = 100.0f
        };
        m_camera->Add<mgv::Camera>(cameraCreateInfo);
    }
}