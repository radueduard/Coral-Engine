//
// Created by radue on 10/24/2024.
//

#include "engine.h"

#include <chrono>

#include "assets/manager.h"
#include "core/input.h"
#include "compute/programs/calculateMvp.h"
#include "compute/programs/fireflies.h"
#include "compute/programs/generateTerrain.h"
#include "graphics/renderer.h"
#include "graphics/programs/firefliesDisplay.h"
#include "graphics/programs/lake.h"
#include "graphics/programs/skyBox.h"
#include "graphics/programs/terrain.h"
#include "renderPasses/mainViewport.h"
#include "renderPasses/reflectionPass.h"

boost::random::mt19937 Utils::Random::m_rng = boost::random::mt19937(std::random_device()());

namespace mgv {
    Engine::Engine() {
        const auto windowInfo = Core::Window::Info {
            .title = "Mgv",
            .extent = { 1920, 1080 },
            .resizable = true,
            .fullscreen = false
        };

        Core::Window::Init(windowInfo);
        Core::Runtime::Init();
        Core::Device::Init();
        Renderer::Init();
        GUI::Manager::Init();
        Asset::Manager::Init();
    }

    Engine::~Engine() {
        Asset::Manager::Destroy();
        GUI::Manager::Destroy();
        Renderer::Destroy();
        Core::Device::Destroy();
    }

    void Engine::Run() const {
        Core::Input::Setup();

        const auto scene = std::make_unique<Scene>();
        scene->InitUI();
        const auto mainViewport = std::make_unique<MainViewport>();
        mainViewport->InitUI();
        const auto reflectionPass = std::make_unique<ReflectionPass>();

        Renderer::SetMainViewport(mainViewport.get());
        Renderer::SetReflectionPass(reflectionPass.get());

        const auto skyBox = Graphics::CubeMap::Builder()
            .PositiveX("models/textures/cubeMapNight/pos_x.png")
            .NegativeX("models/textures/cubeMapNight/neg_x.png")
            .PositiveY("models/textures/cubeMapNight/pos_y.png")
            .NegativeY("models/textures/cubeMapNight/neg_y.png")
            .PositiveZ("models/textures/cubeMapNight/pos_z.png")
            .NegativeZ("models/textures/cubeMapNight/neg_z.png")
            .Build();

        SkyBox::CreateInfo skyBoxCreateInfo {
            .camera = *scene->Camera().Get<Camera>().value(),
            .cubeMap = *skyBox
        };
        const auto skyBoxProgram = std::make_unique<SkyBox>(mainViewport->RenderPass(), Renderer::DescriptorPool(), skyBoxCreateInfo);
        const auto skyBoxReflection = std::make_unique<SkyBox>(reflectionPass->RenderPass(), Renderer::DescriptorPool(), skyBoxCreateInfo);

        GenerateTerrain::CreateInfo generateTerrainCreateInfo {
            .size = 1024
        };

        const auto generateTerrain = std::make_unique<GenerateTerrain>(Renderer::DescriptorPool(), generateTerrainCreateInfo);
        generateTerrain->Init();
        generateTerrain->InitUI();

        const auto firefliesCreateInfo = Fireflies::CreateInfo {
            .count = 1024,
            .bounds = Graphics::AABB ({ -15.0f, 0.0f, -15.0f }, { 15.0f, 10.0f, 15.0f }),
            .camera = *scene->Camera().Get<Camera>().value(),
            .heightMap = generateTerrain->HeightMap()
        };

        const auto fireflies = std::make_unique<FirefliesDisplay>(mainViewport->RenderPass(), Renderer::DescriptorPool(), firefliesCreateInfo);
        fireflies->Init();

        const Terrain::CreateInfo terrainCreateInfo {
            .camera = *scene->Camera().Get<Camera>().value(),
            .heightMap = generateTerrain->HeightMap(),
            .particlesBuffer = fireflies->ParticlesBuffer(),
            .lightIndicesBuffer = fireflies->LightIndicesBuffer()
        };
        const auto terrain = std::make_unique<Terrain>(mainViewport->RenderPass(), Renderer::DescriptorPool(), terrainCreateInfo);
        terrain->Init();
        terrain->InitUI();

        const auto reflectedTerrain = std::make_unique<Terrain>(reflectionPass->RenderPass(), Renderer::DescriptorPool(), terrainCreateInfo);
        reflectedTerrain->Init();
        reflectedTerrain->InitUI();

        const Lake::CreateInfo lakeCreateInfo {
            .camera = *scene->Camera().Get<Camera>().value(),
            .reflectionPass = reflectionPass->RenderPass(),
            .particlesBuffer = fireflies->ParticlesBuffer(),
            .lightIndicesBuffer = fireflies->LightIndicesBuffer()
        };

        const auto lake = std::make_unique<Lake>(mainViewport->RenderPass(), Renderer::DescriptorPool(), lakeCreateInfo);
        lake->Init();
        lake->InitUI();

        while (!Core::Window::Get().ShouldClose()) {
            Core::Window::PollEvents();
            Core::Window::Get().UpdateDeltaTime();
            if (!Core::Window::Get().IsPaused()) {
                const double start = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                const auto viewportSize = mainViewport->RenderPass().Extent();
                Camera::mainCamera->Resize({ viewportSize.width, viewportSize.height });
                scene->Update(Core::Window::Get().DeltaTime());
                GUI::Manager::Update();

                if (Renderer::BeginFrame()) {
                    Renderer::Update(Core::Window::Get().DeltaTime());
                    Renderer::Draw();
                    Renderer::EndFrame();
                }
                const double end = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                Core::Window::Get().SetTitle("Mgv - " + std::to_string(1.0 / (end - start)) + " fps");
            }

            if (Core::Input::IsKeyPressed(Escape)) {
                Core::Window::Get().Close();
            }
            Core::Input::Update();
        }
        (*Core::Device::Get()).waitIdle();
        terrain->DestroyUI();
        reflectedTerrain->DestroyUI();
        lake->DestroyUI();
        generateTerrain->DestroyUI();
        scene->DestroyUI();
        mainViewport->DestroyUI();
        Memory::Sampler::FreeAllSamplers();
    }
}
