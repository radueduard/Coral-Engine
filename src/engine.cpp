//
// Created by radue on 10/24/2024.
//

#include "engine.h"

#include <chrono>

#include "core/input.h"
#include "programs/circleProgram.h"

namespace mgv {
    Engine::Engine() {
        const auto windowInfo = Core::Window::Info {
            .title = "Mgv",
            .extent = { 800, 600 },
            .resizable = true,
            .fullscreen = false
        };

        m_window = std::make_unique<Core::Window>(windowInfo);
        m_runtime = std::make_unique<Core::Runtime>(*m_window);
        m_device = std::make_unique<Core::Device>(m_runtime->PhysicalDevice());
        m_renderer = std::make_unique<Graphics::Renderer>(*m_window, *m_device);
    }

    void Engine::Run() const {
        Core::Input::Setup();

        CircleProgram circleProgram(*m_device, *m_renderer, 200);
        circleProgram.Init();

        while (!m_window->ShouldClose()) {
            Core::Window::PollEvents();
            if (!m_window->IsPaused()) {
                const double start = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

                circleProgram.Update();

                const auto drawFunc = [&circleProgram](const vk::CommandBuffer commandBuffer) {
                    circleProgram.Draw(commandBuffer);
                };

                if (m_renderer->BeginFrame()) {
                    m_renderer->Draw(drawFunc);
                    m_renderer->EndFrame();
                }
                const double end = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                m_window->SetTitle("Mgv - " + std::to_string(1.0 / (end - start)) + " fps");
            }

            if (Core::Input::IsKeyPressed(Escape)) {
                m_window->Close();
            }
            Core::Input::Update();
        }
    }
}
