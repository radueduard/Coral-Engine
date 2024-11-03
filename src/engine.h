//
// Created by radue on 10/24/2024.
//

#pragma once
#include <memory>

#include "core/device.h"
#include "core/window.h"
#include "core/runtime.h"
#include "graphics/renderer.h"

namespace mgv {

    class Engine {
    public:
        Engine();
        ~Engine() = default;

        void Run() const;
    private:
        std::unique_ptr<Core::Window> m_window;
        std::unique_ptr<Core::Runtime> m_runtime;
        std::unique_ptr<Core::Device> m_device;
        std::unique_ptr<Graphics::Renderer> m_renderer;
    };

}
