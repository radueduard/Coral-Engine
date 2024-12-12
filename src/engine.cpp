//
// Created by radue on 10/24/2024.
//

#include "engine.h"

#include <chrono>

#include "assets/manager.h"
#include "core/input.h"
#include "compute/programs/fireflies.h"
#include "compute/programs/generateTerrain.h"
#include "renderer.h"
#include "components/camera.h"
#include "components/object.h"
#include "compute/programs/partitionLights.h"
#include "core/runtime.h"
#include "core/window.h"
#include "graphics/objects/textureArray.h"
#include "graphics/programs/displayViewFrustums.h"
#include "graphics/programs/firefliesDisplay.h"
#include "graphics/programs/lake.h"
#include "graphics/programs/terrain.h"
#include "graphics/programs/trajectoryDisplay.h"
#include "memory/buffer.h"
#include "memory/manager.h"
#include "memory/sampler.h"
#include "scene/scene.h"

namespace mgv {
    Engine::Engine() {
        const auto windowInfo = Core::Window::Info {
            .title = "Mgv",
            .extent = { 2500, 1200 },
            .resizable = true,
            .fullscreen = false
        };

        Core::Window::Init(windowInfo);
        Core::Runtime::Init();
        Core::Device::Init();
        Renderer::Init();
        GUI::Manager::Init();
        Memory::Manager::Init();
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
        scene->Init();
        scene->OnUIAttach();

        // Resources
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

        const auto particlesBuffer = std::make_unique<Memory::Buffer>(
            sizeof(Fireflies::Particle), 1024,
            vk::BufferUsageFlagBits::eStorageBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        const auto bounds = Graphics::AABB ({ -15.0f, 0.0f, -15.0f }, { 15.0f, 10.0f, 15.0f });
        particlesBuffer->Map<Fireflies::Particle>();
        for (uint32_t i = 0; i < 1024; i++) {
            particlesBuffer->WriteAt(i, Fireflies::Particle::Random(bounds));
        }
        particlesBuffer->Flush();
        particlesBuffer->Unmap();

        const auto lightTrajectoriesBuffer = std::make_unique<Memory::Buffer>(
            sizeof(Fireflies::BezierTrajectory), 1024,
            vk::BufferUsageFlagBits::eStorageBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        auto trajectories = lightTrajectoriesBuffer->Map<Fireflies::BezierTrajectory>();
        for (uint32_t i = 0; i < 1024; i++) {
            Fireflies::BezierTrajectory trajectory{};
            for (uint32_t j = 0; j < 4; j++) {
                glm::vec3 pos = Utils::Random::Direction();
                trajectory.onCurvePoints[j] = glm::vec4(pos, 1.0f);

                glm::vec3 center = trajectory.onCurvePoints[j];
                glm::vec3 tangent = cross(center, Utils::Random::Direction());
                glm::vec3 controlPoint1 = center + tangent;
                glm::vec3 controlPoint2 = center - tangent;
                trajectory.controlPoints[j * 2] = glm::vec4(controlPoint1, 1.0f);
                trajectory.controlPoints[j * 2 + 1] = glm::vec4(controlPoint2, 1.0f);
            }
            trajectories[i] = trajectory;
        }
        lightTrajectoriesBuffer->Flush();
        lightTrajectoriesBuffer->Unmap();

        const auto lightIndicesBuffer = std::make_unique<Memory::Buffer>(
            sizeof(Indices), 64 * 64,
            vk::BufferUsageFlagBits::eStorageBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        lightIndicesBuffer->Map<Indices>();
        for (uint32_t i = 0; i < 64 * 64; i++) {
            Indices indices{};
            for (uint32_t j = 0; j < 64; j++) {
                indices.indices[j] = j;
            }
            lightIndicesBuffer->WriteAt(i, indices);
        }
        lightIndicesBuffer->Flush();
        lightIndicesBuffer->Unmap();

        GenerateTerrain::CreateInfo generateTerrainCreateInfo {
            .size = 2048,
            .albedoTextures = *albedoTextures,
            .normalTextures = *normalTextures
        };

        const auto generateTerrain = std::make_unique<GenerateTerrain>(generateTerrainCreateInfo);
        generateTerrain->Init();

        const auto firefliesCreateInfo = FirefliesDisplay::CreateInfo {
            .heightMap = generateTerrain->HeightMap(),
            .particlesBuffer = *particlesBuffer,
            .lightIndicesBuffer = *lightIndicesBuffer,
            .trajectoriesBuffer = *lightTrajectoriesBuffer,
            .frustumsBuffer = *Camera::Main()->FrustumsBuffer()
        };

        const auto trajectoryDisplayCreateInfo = TrajectoryDisplay::CreateInfo {
            .particlesBuffer = *particlesBuffer,
            .trajectoriesBuffer = *lightTrajectoriesBuffer,
            .heightMap = generateTerrain->HeightMap()
        };

        const auto trajectoryDisplay = std::make_unique<TrajectoryDisplay>(trajectoryDisplayCreateInfo);

        const auto displayViewFrustumsCreateInfo = DisplayViewFrustums::CreateInfo {
            .frustumsBuffer = *Camera::Main()->FrustumsBuffer()
        };
        const auto displayViewFrustums = std::make_unique<DisplayViewFrustums>(displayViewFrustumsCreateInfo);

        const auto fireflies = std::make_unique<FirefliesDisplay>(firefliesCreateInfo);
        fireflies->Init();

        Terrain::CreateInfo terrainCreateInfo {
            .heightMap = generateTerrain->HeightMap(),
            .albedo = generateTerrain->Albedo(),
            .normal = generateTerrain->Normal(),
            .particlesBuffer = *particlesBuffer,
            .lightIndicesBuffer = *lightIndicesBuffer
        };
        const auto terrain = std::make_unique<Terrain>(terrainCreateInfo);
        terrain->Init();

        Lake::CreateInfo lakeCreateInfo {
            .particlesBuffer = *particlesBuffer,
            .lightIndicesBuffer = *lightIndicesBuffer
        };

        const auto lake = std::make_unique<Lake>(lakeCreateInfo);
        lake->Init();

        while (!Core::Window::ShouldClose()) {
            Core::Window::PollEvents();
            Core::Window::UpdateDeltaTime();
            if (!Core::Window::IsPaused()) {
                const double start = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                const auto viewportSize = Renderer::Extent();
                Camera::Main()->Resize({ viewportSize.width, viewportSize.height });
                GUI::Manager::Update();
                scene->Update(Core::Window::DeltaTime());

                if (Renderer::BeginFrame()) {
                    generateTerrain->Compute();
                    Renderer::Update(Core::Window::DeltaTime());
                    Renderer::Draw();
                    Renderer::EndFrame();
                }

                scene->LateUpdate(Core::Window::DeltaTime());
                const double end = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                Core::Window::SetTitle("Mgv - " + std::to_string(1.0 / (end - start)) + " fps");
            }

            if (Core::Input::IsKeyPressed(Escape)) {
                Core::Window::Close();
            }
            Core::Input::Update();
        }
        (*Core::Device::Get()).waitIdle();

        GUI::Manager::Destroy();

        Memory::Sampler::FreeAllSamplers();
    }
}
