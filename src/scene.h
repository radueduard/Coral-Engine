//
// Created by radue on 10/24/2024.
//

#pragma once
#include <memory>

#include "components/camera.h"
#include "components/object.h"

namespace mgv {

    class Scene {
    public:
        Scene();

        [[nodiscard]] Object& Root() const { return *m_root; }
        [[nodiscard]] Object& Camera() const { return *m_camera; }
    private:
        std::unique_ptr<Object> m_root;
        std::unique_ptr<Object> m_camera;
    };

}
