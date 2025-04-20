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
#include "project/scene.h"
#include "shader/manager.h"

import types;
import math.matrix;
import math.vector;

namespace Coral {
    void func() {
        const Math::Matrix2<f32> a;
        glm::mat4 b = glm::mat4(a);
        std::cout << a << std::endl;
        std::cout << Math::Matrix2<f32>(b) << std::endl;
    }
}
Engine::Engine() {
    Coral::func();

    const auto windowCreateInfo = Core::Window::CreateInfo {
        .title = "Coral",
        .extent = { 1920, 1080 },
        .resizable = true,
        .fullscreen = false
    };

    m_window = std::make_unique<Core::Window>(windowCreateInfo);

    m_scriptingDomain = std::unique_ptr<Coral::Scripting::Domain>(new Coral::Scripting::Domain("RootDomain"));

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

    Asset::Manager::Init();
}

Engine::~Engine() {
    Asset::Manager::Destroy();
    Coral::Scripting::Domain::JitCleanup()(m_scriptingDomain.get());
}

void Engine::Run() const {
    Core::Input::Setup();

    GUI::Container<Asset::Manager> assetManager = GUI::MakeContainer<Asset::Manager>();
    GUI::Container<Shader::Manager> shaderManager = GUI::MakeContainer<Shader::Manager>(std::filesystem::path("shaders"));

    Asset::Importer importer("assets/DamagedHelmet/DamagedHelmet.gltf");
    // Asset::Importer importer("assets/main1_sponza/NewSponza_Main_glTF_003.gltf");
    importer.Import();
    GUI::Container<Coral::Scene> scene = importer.LoadScene();

    while (!m_window->ShouldClose()) {
        m_window->PollEvents();
        m_window->UpdateDeltaTime();
        if (!m_window->IsPaused()) {
            m_scheduler->Update(m_window->DeltaTime());
            m_scheduler->Draw();
        }

        if (Core::Input::IsKeyPressed(Key::Esc)) {
            m_window->Close();
        }

        Core::Input::Update();
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    }
    Core::GlobalDevice()->waitIdle();
}
