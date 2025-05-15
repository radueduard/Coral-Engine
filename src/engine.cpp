//
// Created by radue on 10/24/2024.
//
#include <chrono>

#include "core/window.h"
#include "core/input.h"
#include "core/runtime.h"
#include "core/device.h"
#include "core/physicalDevice.h"
#include "core/scheduler.h"

#include "engine.h"

#include "assets/importer.h"
#include "assets/manager.h"
#include "gui/container.h"
#include "ecs/scene.h"
#include "shader/manager.h"

#include "math/matrix.h"

namespace Coral {
    Engine::Engine() {
        const auto windowCreateInfo = Core::Window::CreateInfo {
            .title = "Coral",
            .extent = { 1920, 1080 },
            .resizable = true,
            .fullscreen = false
        };

        m_window = std::make_unique<Core::Window>(windowCreateInfo);

        m_scriptingDomain = std::unique_ptr<Scripting::Domain>(new Scripting::Domain("RootDomain"));

        const auto runtimeCreateInfo = Core::Runtime::CreateInfo {
            .window = *m_window,
            .deviceFeatures = vk::PhysicalDeviceFeatures()
            .setSamplerAnisotropy(true)
            .setFragmentStoresAndAtomics(true)
            .setFillModeNonSolid(true)
            .setVertexPipelineStoresAndAtomics(true),
            .instanceLayers = {
                "VK_LAYER_KHRONOS_validation",
            },
            .instanceExtensions = {
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME
            },
            .deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_EXT_MESH_SHADER_EXTENSION_NAME,
            },
            .deviceLayers = {
                "VK_LAYER_KHRONOS_validation",
            },
            .requiredQueueFamilies = {
                vk::QueueFlagBits::eGraphics,
                vk::QueueFlagBits::eCompute,
                vk::QueueFlagBits::eTransfer,
            },
        };

        m_runtime = std::make_unique<Core::Runtime>(runtimeCreateInfo);

        const auto deviceCreateInfo = Core::Device::CreateInfo {
            .runtime = *m_runtime,
        };

        m_device = std::make_unique<Core::Device>(deviceCreateInfo);
        Core::g_device = m_device.get();

        const auto schedulerCreateInfo = Core::Scheduler::CreateInfo {
            .window = *m_window,
            .runtime = *m_runtime,
            .minImageCount = m_runtime->PhysicalDevice().SurfaceCapabilities().minImageCount,
            .imageCount = 3,
            .multiSampling = vk::SampleCountFlagBits::e2,
        };

        m_scheduler = std::make_unique<Core::Scheduler>(schedulerCreateInfo);
    	m_assetManager = std::make_unique<Asset::Manager>();
    }

    Engine::~Engine() {
        Scripting::Domain::JitCleanup()(m_scriptingDomain.get());
    }

    void Engine::Run() const {
        Input::Setup();

        Reef::Container<Asset::Manager> assetManager = Reef::MakeContainer<Asset::Manager>();
        Reef::Container<Shader::Manager> shaderManager = Reef::MakeContainer<Shader::Manager>(std::filesystem::path("shaders"));

        Asset::Importer importer("assets/DamagedHelmet/DamagedHelmet.gltf");
        // Asset::Importer importer("assets/main1_sponza/NewSponza_Main_glTF_003.gltf");

        importer.Import();
        Reef::Container<ECS::Scene> scene = importer.LoadScene();

        while (!m_window->ShouldClose()) {
            auto startTime = std::chrono::high_resolution_clock::now();
            m_window->PollEvents();
            m_window->UpdateDeltaTime();
            if (!m_window->IsPaused()) {
                m_scheduler->Update(m_window->DeltaTime());
                m_scheduler->Draw();
            }

            if (Input::IsKeyPressed(Key::Esc)) {
                m_window->Close();
            }

            Input::Update();
            auto end = std::chrono::high_resolution_clock::now();
            const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - startTime).count();
            // std::cout << "Frame time: " << static_cast<float>(elapsed) / 1000.f << "ms" << std::endl;
            m_window->SetTitle("Coral - " + std::to_string(1000000.f / static_cast<float>(elapsed)) + "fps");
        }
        Core::GlobalDevice()->waitIdle();
    }
}
