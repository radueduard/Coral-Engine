//
// Created by radue on 10/24/2024.
//

#include "engine.h"

#include <chrono>

#include "assets/importer.h"
#include "assets/manager.h"
#include "core/input.h"
#include "graphics/programs/displayNormals.h"
#include "compute/programs/calculateMvp.h"
#include "graphics/programs/lake.h"
#include "graphics/programs/terrain.h"
#include "memory/sampler.h"

namespace mgv {
    Engine::Engine() {
        const auto windowInfo = Core::Window::Info {
            .title = "Mgv",
            .extent = { 1920, 1080 },
            .resizable = true,
            .fullscreen = false
        };

        m_window = std::make_unique<Core::Window>(windowInfo);
        m_runtime = std::make_unique<Core::Runtime>(*m_window);
        m_device = std::make_unique<Core::Device>(*m_runtime);

        Renderer::Init(*m_window, *m_device);
        GUI::Manager::Init(*Renderer::Instance());
        Asset::Manager::Init(*m_device);
    }

    Engine::~Engine() {
        Asset::Manager::Destroy();
        GUI::Manager::Destroy();
        Renderer::Destroy();
    }

    void Engine::Run() const {
        Core::Input::Setup();

        const auto scene = std::make_unique<Scene>(*m_device);
        scene->InitUI();
        const auto mainViewport = std::make_unique<MainViewport>(*m_device);
        mainViewport->InitUI();

        Renderer::SetMainViewport(mainViewport.get());

        const Terrain::CreateInfo terrainCreateInfo {
            .camera = *scene->Camera().Get<Camera>().value()
        };

        const Lake::CreateInfo lakeCreateInfo {
            .camera = *scene->Camera().Get<Camera>().value()
        };

        auto terrain = std::make_unique<Terrain>(*m_device, mainViewport->RenderPass(), Renderer::DescriptorPool(), terrainCreateInfo);
        terrain->Init();
        terrain->InitUI();

        // auto lake = std::make_unique<Lake>(*m_device, mainViewport->RenderPass(), Renderer::DescriptorPool(), lakeCreateInfo);
        // lake->Init();
        // lake->InitUI();

        while (!m_window->ShouldClose()) {
            Core::Window::PollEvents();
            m_window->UpdateDeltaTime();
            if (!m_window->IsPaused()) {
                const double start = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                const auto viewportSize = mainViewport->RenderPass().Extent();
                Camera::mainCamera->Resize({ viewportSize.width, viewportSize.height });
                scene->Update(m_window->DeltaTime());

                GUI::Manager::Update();

                if (Renderer::BeginFrame()) {
                    Renderer::Draw();
                    Renderer::EndFrame();
                }
                const double end = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                m_window->SetTitle("Mgv - " + std::to_string(1.0 / (end - start)) + " fps");
            }

            if (Core::Input::IsKeyPressed(Escape)) {
                m_window->Close();
            }

            if (Core::Input::IsKeyPressed(F5)) {
                (**m_device).waitIdle();
                terrain->DestroyUI();
                // lake->DestroyUI();
                terrain = std::make_unique<Terrain>(*m_device, mainViewport->RenderPass(), Renderer::DescriptorPool(), terrainCreateInfo);
                // lake = std::make_unique<Lake>(*m_device, mainViewport->RenderPass(), Renderer::DescriptorPool(), lakeCreateInfo);
                terrain->Init();
                terrain->InitUI();
                // lake->Init();
                // lake->InitUI();
            }
            Core::Input::Update();
        }
        (**m_device).waitIdle();
        terrain->DestroyUI();
        // lake->DestroyUI();
        scene->DestroyUI();
        mainViewport->DestroyUI();
        Memory::Sampler::FreeAllSamplers();
    }
}
