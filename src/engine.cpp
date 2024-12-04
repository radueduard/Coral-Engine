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

    void Engine::Run() {
        Core::Input::Setup();

        const auto scene = std::make_unique<Scene>();
        scene->InitUI();
        const auto mainViewport = std::make_unique<MainViewport>();
        mainViewport->InitUI();
        const auto reflectionPass = std::make_unique<ReflectionPass>();

        Renderer::SetMainViewport(mainViewport.get());
        Renderer::SetReflectionPass(reflectionPass.get());

        // Resources
        const auto skyBox = Graphics::CubeMap::Builder()
            .PositiveX("models/textures/cubeMapNight/pos_x.png")
            .NegativeX("models/textures/cubeMapNight/neg_x.png")
            .PositiveY("models/textures/cubeMapNight/pos_y.png")
            .NegativeY("models/textures/cubeMapNight/neg_y.png")
            .PositiveZ("models/textures/cubeMapNight/pos_z.png")
            .NegativeZ("models/textures/cubeMapNight/neg_z.png")
            .Build();

        const auto albedoTextures = TextureArray::Builder()
            .Name("Albedo")
            .Format(vk::Format::eR8G8B8A8Srgb)
            .ImageSize(2048)
            .AddImagePath("models/textures/wavy_sand/wavy-sand_albedo.png")
            .AddImagePath("models/textures/stylized_grass/stylized-grass1_albedo.png")
            .AddImagePath("models/textures/stylized_cliff/stylized-cliff1-albedo.png")
            .AddImagePath("models/textures/snow_packed/snow-packed12-Base_Color.png")
            .CreateMipmaps()
            .Build();

        const auto normalTextures = TextureArray::Builder()
            .Name("Normal")
            .Format(vk::Format::eR8G8B8A8Unorm)
            .ImageSize(2048)
            .AddImagePath("models/textures/wavy_sand/wavy-sand_normal-dx.png")
            .AddImagePath("models/textures/stylized_grass/stylized-grass1_normal-dx.png")
            .AddImagePath("models/textures/stylized_cliff/stylized-cliff1-normal-dx.png")
            .AddImagePath("models/textures/snow_packed/snow-packed12-Normal-dx.png")
            .CreateMipmaps()
            .Build();

        const auto particlesBuffer = std::make_unique<Memory::Buffer<Fireflies::Particle>>(
            1024,
            vk::BufferUsageFlagBits::eStorageBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        const auto bounds = Graphics::AABB ({ -15.0f, 0.0f, -15.0f }, { 15.0f, 10.0f, 15.0f });
        particlesBuffer->Map();
        for (uint32_t i = 0; i < 1024; i++) {
            particlesBuffer->WriteAt(i, Fireflies::Particle::Random(bounds));
        }
        particlesBuffer->Flush();
        particlesBuffer->Unmap();

        const auto lightIndicesBuffer = std::make_unique<Memory::Buffer<Indices>>(
            30 * 30,
            vk::BufferUsageFlagBits::eStorageBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        // Programs
        SkyBox::CreateInfo skyBoxCreateInfo {
            .camera = *scene->Camera().Get<Camera>().value(),
            .cubeMap = *skyBox
        };
        const auto skyBoxProgram = std::make_unique<SkyBox>(mainViewport->RenderPass(), Renderer::DescriptorPool(), skyBoxCreateInfo);
        const auto skyBoxReflection = std::make_unique<SkyBox>(reflectionPass->RenderPass(), Renderer::DescriptorPool(), skyBoxCreateInfo);

        GenerateTerrain::CreateInfo generateTerrainCreateInfo {
            .size = 1024,
            .albedoTextures = *albedoTextures,
            .normalTextures = *normalTextures
        };

        const auto generateTerrain = std::make_unique<GenerateTerrain>(Renderer::DescriptorPool(), generateTerrainCreateInfo);
        generateTerrain->Init();
        generateTerrain->InitUI();


        const auto firefliesCreateInfo = FirefliesDisplay::CreateInfo {
            .camera = *scene->Camera().Get<Camera>().value(),
            .heightMap = generateTerrain->HeightMap(),
            .particlesBuffer = *particlesBuffer,
            .lightIndicesBuffer = *lightIndicesBuffer
        };

        const auto fireflies = std::make_unique<FirefliesDisplay>(mainViewport->RenderPass(), Renderer::DescriptorPool(), firefliesCreateInfo);
        fireflies->Init();

        const Terrain::CreateInfo terrainCreateInfo {
            .camera = *scene->Camera().Get<Camera>().value(),
            .heightMap = *generateTerrain->HeightMap(),
            .albedo = *generateTerrain->Albedo(),
            .normal = *generateTerrain->Normal(),
            .particlesBuffer = *particlesBuffer,
            .lightIndicesBuffer = *lightIndicesBuffer
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
            .particlesBuffer = *particlesBuffer,
            .lightIndicesBuffer = *lightIndicesBuffer
        };

        const auto lake = std::make_unique<Lake>(mainViewport->RenderPass(), Renderer::DescriptorPool(), lakeCreateInfo);
        lake->Init();
        lake->InitUI();

        while (!Core::Window::ShouldClose()) {
            Core::Window::PollEvents();
            Core::Window::UpdateDeltaTime();
            if (!Core::Window::IsPaused()) {
                const double start = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                const auto viewportSize = mainViewport->RenderPass().Extent();
                Camera::mainCamera->Resize({ viewportSize.width, viewportSize.height });
                scene->Update(Core::Window::DeltaTime());
                GUI::Manager::Update();

                if (Renderer::BeginFrame()) {
                    Renderer::Update(Core::Window::DeltaTime());
                    Renderer::Draw();
                    Renderer::EndFrame();
                }
                const double end = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                Core::Window::SetTitle("Mgv - " + std::to_string(1.0 / (end - start)) + " fps");
            }

            if (Core::Input::IsKeyPressed(Escape)) {
                Core::Window::Close();
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
