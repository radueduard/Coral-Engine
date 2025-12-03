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
#include "ecs/scene.h"
#include "gui/container.h"
#include "shader/manager.h"

#include "gui/elements/popup.h"

namespace Coral {
    Engine::Engine() {
        const auto windowCreateInfo = Core::Window::CreateInfo {
            .title = "Coral",
            .extent = { 1920u, 1080u },
            .resizable = true,
            .fullscreen = false
        };

        m_window = std::make_unique<Core::Window>(windowCreateInfo);

        const auto runtimeCreateInfo = Core::Runtime::CreateInfo {
            .deviceFeatures = vk::PhysicalDeviceFeatures()
	            .setSamplerAnisotropy(true)
	            .setFragmentStoresAndAtomics(true)
	            .setFillModeNonSolid(true)
        		.setTessellationShader(true)
				.setGeometryShader(true)
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
        m_device = std::make_unique<Core::Device>();

    	m_shaderManager = std::make_unique<Shader::Manager>(std::filesystem::path("shaders"));
        const auto schedulerCreateInfo = Core::Scheduler::CreateInfo {
            .minImageCount = m_runtime->PhysicalDevice().SurfaceCapabilities().minImageCount,
            .imageCount = 3,
            .multiSampling = vk::SampleCountFlagBits::e2,
        };

        m_scheduler = std::make_unique<Core::Scheduler>(schedulerCreateInfo);
    	m_sceneManager = std::make_unique<ECS::SceneManager>();
    	m_assetManager = Reef::MakeContainer<Asset::Manager>();
    }

    void Engine::Run() const {
        Input::Setup();

		Asset::Importer("C:/Users/radue/CLionProjects/Coral-Engine/assets/DamagedHelmet/DamagedHelmet.gltf").Import();

        while (!m_window->ShouldClose()) {
	        auto startTime = std::chrono::high_resolution_clock::now();
        	m_window->PollEvents();
        	m_window->UpdateDeltaTime();
        	m_shaderManager->Update();
        	m_sceneManager->Update(m_window->DeltaTime());

            if (!m_window->IsPaused()) {
            	if (ECS::SceneManager::Get().IsSceneLoaded())
					m_sceneManager->GetLoadedScene().Update(m_window->DeltaTime());
                m_scheduler->Update(m_window->DeltaTime());
                m_scheduler->Draw();
            }

        	m_shaderManager->LateUpdate();

            Input::Update();

            auto end = std::chrono::high_resolution_clock::now();
            const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - startTime).count();
            m_window->SetTitle("Coral - " + std::to_string(1000000.f / static_cast<float>(elapsed)) + "fps");
        }
        Context::Device()->waitIdle();
    }
}

