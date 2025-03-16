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
#include "compute/pipeline.h"
#include "graphics/pipeline.h"
#include "graphics/renderPass.h"
#include "graphics/objects/mesh.h"
#include "gui/container.h"
#include "gui/manager.h"
#include "gui/viewport.h"
#include "project/scene.h"
#include "shader/manager.h"

Engine::Engine() {
    const auto windowCreateInfo = Core::Window::CreateInfo {
        .title = "Motor Grafic Vulkan",
        .extent = { 1920, 1080 },
        .resizable = true,
        .fullscreen = false
    };

    m_window = std::make_unique<Core::Window>(windowCreateInfo);

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

    const auto schedulerCreateInfo = Core::Scheduler::CreateInfo {
        .window = *m_window,
        .runtime = *m_runtime,
        .minImageCount = m_runtime->PhysicalDevice().SurfaceCapabilities().minImageCount,
        .imageCount = 3,
        .multiSampling = vk::SampleCountFlagBits::e2,
    };

    m_scheduler = std::make_unique<Core::Scheduler>(schedulerCreateInfo);
}

void Engine::Run() const {
    Core::Input::Setup();

    auto shader = Core::Shader("projects/testProject/shaders/test.vert");

    GUI::Container<Asset::Manager> assetManager = GUI::MakeContainer<Asset::Manager>();
    GUI::Container<mgv::Scene> scene = GUI::MakeContainer<mgv::Scene>();
    GUI::Container<Shader::Manager> shaderManager = GUI::MakeContainer<Shader::Manager>(std::filesystem::path("shaders"));

    const Asset::Importer importer("assets/main1_sponza/NewSponza_Main_glTF_003.gltf");

    // auto func = [&importer] {
    //     Core::GlobalDevice().CreateCommandPools(std::this_thread::get_id()._Get_underlying_id());
    //     importer.LoadTextures();
    //     Core::GlobalDevice().FreeCommandPools(std::this_thread::get_id()._Get_underlying_id());
    // };
    //
    // std::thread threads[1];
    // for (auto& t : threads) {
    //     t = std::thread(func);
    // }

    while (!m_window->ShouldClose()) {
        m_window->PollEvents();
        m_window->UpdateDeltaTime();
        if (!m_window->IsPaused()) {
            m_scheduler->Update(m_window->DeltaTime());
            m_scheduler->Draw();
        }

        if (Core::Input::IsKeyPressed(Esc)) {
            m_window->Close();
        }

        Core::Input::Update();
    }
    Core::GlobalDevice()->waitIdle();
}
