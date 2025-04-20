//
// Created by radue on 4/18/2025.
//

export module engine;

import <vulkan/vulkan.hpp>;

import input;

import core.window;
import core.runtime;
import core.device;

import std;

namespace Coral {
	export class Engine {
	public:
		Engine() {
			const auto windowCreateInfo = Core::Window::CreateInfo {
				.title = "Coral",
				.extent = { 1920, 1080 },
				.resizable = true,
				.fullscreen = false
			};

			m_window = std::make_unique<Core::Window>(windowCreateInfo);

			// m_scriptingDomain = std::unique_ptr<Coral::Scripting::Domain>(new Coral::Scripting::Domain("RootDomain"));

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

			m_device = std::make_unique<Core::Device>();

			// const auto schedulerCreateInfo = Core::Scheduler::CreateInfo {
			// 	.window = *m_window,
			// 	.runtime = *m_runtime,
			// 	.minImageCount = m_runtime->PhysicalDevice().SurfaceCapabilities().minImageCount,
			// 	.imageCount = 3,
			// 	.multiSampling = vk::SampleCountFlagBits::e2,
			// };
			//
			// m_scheduler = std::make_unique<Core::Scheduler>(schedulerCreateInfo);

			// Asset::Manager::Init();
		}
		~Engine() {
			// Asset::Manager::Destroy();
			// Coral::Scripting::Domain::JitCleanup()(m_scriptingDomain.get());
		}

		void Run() const {
			Input::Setup();

			// GUI::Container<Asset::Manager> assetManager = GUI::MakeContainer<Asset::Manager>();
			// GUI::Container<Shader::Manager> shaderManager = GUI::MakeContainer<Shader::Manager>(std::filesystem::path("shaders"));

			// Asset::Importer importer("assets/DamagedHelmet/DamagedHelmet.gltf");
			// Asset::Importer importer("assets/main1_sponza/NewSponza_Main_glTF_003.gltf");
			// importer.Import();
			// GUI::Container<Coral::Scene> scene = importer.LoadScene();

			while (!m_window->ShouldClose()) {
				m_window->PollEvents();
				m_window->UpdateDeltaTime();
				if (!m_window->IsPaused()) {
					// m_scheduler->Update(m_window->DeltaTime());
					// m_scheduler->Draw();
				}

				if (Input::IsKeyPressed(Key::Esc)) {
					m_window->Close();
				}

				Input::Update();
				std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
			}
			Core::GlobalDevice()->waitIdle();
		}
	private:
		std::unique_ptr<Core::Window> m_window;
		// std::unique_ptr<Coral::Scripting::Domain> m_scriptingDomain;
		std::unique_ptr<Core::Runtime> m_runtime;
		std::unique_ptr<Core::Device> m_device;
		// std::unique_ptr<Core::Scheduler> m_scheduler;
	};
}
