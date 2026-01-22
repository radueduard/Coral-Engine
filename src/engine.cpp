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
#include "assets/manager.h"
#include "ecs/sceneManager.h"
#include "shader/manager.h"

#include "engine.h"

#include "planeMeshes.h"

#include "assets/importer.h"
#include "compute/program.h"
#include "compute/programs/generateTextureMesh.h"

#include "core/time.h"

#include "ecs/scene.h"
#include "ecs/components/renderTarget.h"

#include "graphics/objects/baseMeshes.h"

#include "gui/container.h"

#include "gui/elements/popup.h"
#include "shader/slangCompiler.h"
#include "utils/fileSystemObserver.h"
#include "utils/noise.h"

namespace Coral {
	class ImageTest : public Reef::Layer {
	public:
		explicit ImageTest(const Memory::Image& image) : m_image(image) {
			m_imageView = Memory::ImageView::Builder(image)
				.ViewType(vk::ImageViewType::e2D)
				.Build();

			m_sampler = Memory::Sampler::Builder()
				.Build();

			m_textureID = ImGui_ImplVulkan_AddTexture(
				**m_sampler,
				**m_imageView,
				static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
		}

    	void OnGUIAttach() override {
			AddDockable("Test Image", new Reef::Window(
				"Test Image",
				{},
				{
					new Reef::Image((m_textureID))
				},
				nullptr));
    	}
    private:
    	const Memory::Image& m_image;
    	std::unique_ptr<Memory::ImageView> m_imageView;
    	std::unique_ptr<Memory::Sampler> m_sampler;
    	ImTextureID m_textureID;
    };

    void Engine::Run() const {
    	std::unique_ptr<Utils::FileSystemObserver> m_fileSystemObserver = nullptr;
    	std::unique_ptr<Shader::SlangCompiler> m_slangCompiler = nullptr;

    	std::unique_ptr<Core::Window> m_window = nullptr;
    	std::unique_ptr<Core::Runtime> m_runtime = nullptr;
    	std::unique_ptr<Core::Device> m_device = nullptr;
    	std::unique_ptr<Shader::Manager> m_shaderManager = nullptr;
    	std::unique_ptr<Core::Scheduler> m_scheduler = nullptr;
    	std::unique_ptr<ECS::SceneManager> m_sceneManager = nullptr;
    	Reef::Container<Asset::Manager> m_assetManager;

		m_fileSystemObserver = std::make_unique<Utils::FileSystemObserver>();

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
        		.setShaderStorageImageReadWithoutFormat(true)
        		.setShaderStorageImageWriteWithoutFormat(true)
				// .setGeometryShader(true)
	            .setVertexPipelineStoresAndAtomics(true),
            .instanceLayers = {
                "VK_LAYER_KHRONOS_validation",
            },
            .instanceExtensions = {
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            },
            .deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            	VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                // VK_EXT_MESH_SHADER_EXTENSION_NAME,
            	// VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME
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

    	m_shaderManager = std::make_unique<Shader::Manager>();
        const auto schedulerCreateInfo = Core::Scheduler::CreateInfo {
            .minImageCount = m_runtime->PhysicalDevice().SurfaceCapabilities().minImageCount,
            .imageCount = 3,
            .multiSampling = vk::SampleCountFlagBits::e2,
        };

        m_scheduler = std::make_unique<Core::Scheduler>(schedulerCreateInfo);
    	m_assetManager = Reef::MakeContainer<Asset::Manager>();
    	m_sceneManager = std::make_unique<ECS::SceneManager>();

        Input::Setup();


		// const Utils::PerlinNoise2D noise({ 512u, 512u }, 6);
  //   	Reef::Container<ImageTest> noiseTestContainer = Reef::MakeContainer<ImageTest>(noise.Image());

		const auto image = std::make_unique<Utils::PerlinNoise3D>(Math::Vector3u(256), 9);
		// const auto image = std::make_unique<Utils::CircleNoise<3>>(Math::Vector3u(64u), 0.75f);

    	auto entity = std::make_unique<ECS::Entity>("Generated Planet Mesh");
    	auto& renderTarget = entity->Add<ECS::RenderTarget>();

    	Math::Vector3u chunkCount { 1u, 1u, 1u };
    	chunkCount *= 4u;

  		const Compute::GenerateTextureMesh generateTextureMeshProgram(
  			image->Image(),
			Math::Vector3u { 1u, 1u, 1u } * 8u
  		);

    	const auto& planetMaterial = m_sceneManager->GetLoadedScene().PlanetMaterial();

    	std::vector<std::unique_ptr<Graphics::Mesh>> meshes;

    	for (u32 i = 0; i < chunkCount.x; i++) {
			for (u32 j = 0; j < chunkCount.y; j++) {
				for (u32 k = 0; k < chunkCount.z; k++) {
					auto mesh = generateTextureMeshProgram.Execute(
						Math::Vector3u(i, j, k),
						chunkCount
					);
					if (!mesh) {
						continue;
					}
					renderTarget.Add(mesh.get(), &planetMaterial);
					meshes.emplace_back(std::move(mesh));
				}
			}
		}

    	m_sceneManager->GetLoadedScene().Root().AddChild(std::move(entity));

    	auto waterEntity = std::make_unique<ECS::Entity>("Water Sphere");
		auto& waterRenderTarget = waterEntity->Add<ECS::RenderTarget>();
    	auto* waterMesh = m_assetManager->GetMesh(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000002"));
    	auto& waterMaterial = m_sceneManager->GetLoadedScene().WaterMaterial();
    	waterRenderTarget.Add(waterMesh, &waterMaterial);

    	waterEntity->Get<ECS::Transform>().scale = Math::Vector3f { 15.f };

    	m_sceneManager->GetLoadedScene().Root().AddChild(std::move(waterEntity));
    	m_sceneManager->GetLoadedScene().Root().AddChild(PlaneMeshes::Plane());

    	auto& dirLight = m_sceneManager->GetLoadedScene().Root().AddLight(ECS::LightType::Directional);
    	dirLight.Get<ECS::Transform>().position = Math::Vector3f { 0.f, 0.f, 30.f };

		auto& spot1 = m_sceneManager->GetLoadedScene().Root().AddLight(ECS::LightType::Spot);
    	spot1.Get<ECS::Transform>().position = Math::Vector3f { -2.5f, 0.f, 30.f };
    	spot1.Get<ECS::Transform>().rotation = Math::Vector3f { 0.f, -30.f, 0.f };

    	auto& spot2 = m_sceneManager->GetLoadedScene().Root().AddLight(ECS::LightType::Spot);
    	spot2.Get<ECS::Transform>().position = Math::Vector3f { 2.5f, 0.f, 30.f };
    	spot2.Get<ECS::Transform>().rotation = Math::Vector3f { 0.f, 30.f, 0.f };

		Time::Setup();

    	while (!m_window->ShouldClose()) {
	        auto startTime = std::chrono::high_resolution_clock::now();
        	m_window->PollEvents();
        	Time::Update();

        	m_fileSystemObserver->Update();

        	m_shaderManager->Update();
        	m_sceneManager->Update();
        	// pipeline.Update();

            if (!m_window->IsPaused()) {
            	if (m_sceneManager->IsSceneLoaded())
					m_sceneManager->GetLoadedScene().Update();
                m_scheduler->Update();
    //         	m_device->RunSingleTimeCommand([&](const Core::CommandBuffer& commandBuffer) {
    //         		colorImage->TransitionLayout(commandBuffer, vk::ImageLayout::eGeneral);
    //         		colorImage->Clear(commandBuffer, vk::ClearColorValue(std::array { 0.f, 0.f, 0.f, 1.f }));
				// 	// depthImage->Clear(commandBuffer, vk::ClearDepthStencilValue { 1.f, 0 });
    //         		depthImage->Clear(commandBuffer, vk::ClearColorValue(std::array { UINT32_MAX, 0u, 0u, 0u }));
				// 	pipeline.Bind(commandBuffer);
				// 	pipeline.BindDescriptorSet(0, commandBuffer, *descriptorSet);
				// 	commandBuffer->dispatch(1280 / 16, 720 / 16, 16);
    //         		colorImage->TransitionLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);
				// }, vk::QueueFlagBits::eCompute);

                m_scheduler->Draw();
            }

            Input::Update();

            auto end = std::chrono::high_resolution_clock::now();
            const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - startTime).count();
            m_window->SetTitle("Coral - " + std::to_string(1000000.f / static_cast<float>(elapsed)) + "fps");
        }

        (*m_device)->waitIdle();
    }
}

