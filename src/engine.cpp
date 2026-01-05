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
#include "compute/pipeline.h"
#include "compute/program.h"
#include "compute/programs/bitonicMergeSort.h"
#include "compute/programs/generateTextureMesh.h"
#include "ecs/scene.h"
#include "gui/container.h"
#include "shader/manager.h"

#include "gui/elements/popup.h"
#include "gui/templates/computeProgramTemplate.h"

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
        		.setShaderStorageImageReadWithoutFormat(true)
        		.setShaderStorageImageWriteWithoutFormat(true)
				// .setGeometryShader(true)
	            .setVertexPipelineStoresAndAtomics(true),
            .instanceLayers = std::vector {
                "VK_LAYER_KHRONOS_validation",
            },
            .instanceExtensions = {
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            },
            .deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_EXT_MESH_SHADER_EXTENSION_NAME,
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

    	m_shaderManager = std::make_unique<Shader::Manager>(std::filesystem::path("shaders"));
        const auto schedulerCreateInfo = Core::Scheduler::CreateInfo {
            .minImageCount = m_runtime->PhysicalDevice().SurfaceCapabilities().minImageCount,
            .imageCount = 3,
            .multiSampling = vk::SampleCountFlagBits::e2,
        };

        m_scheduler = std::make_unique<Core::Scheduler>(schedulerCreateInfo);
    	m_assetManager = Reef::MakeContainer<Asset::Manager>();
    	m_sceneManager = std::make_unique<ECS::SceneManager>();
    }

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

	class ProgramSettings : public Reef::Layer {
	public:
		explicit ProgramSettings(Compute::Program& program) : m_program(program) {}

		void OnGUIAttach() override {
			AddDockable("Compute Program Settings",
				new Reef::Window(
					"Compute Program Settings",
					{
						.padding = { 10.f, 10.f, 10.f, 10.f },
					},
					{
						m_programTemplate.Build(m_program)
					}
				)
			);
		}
	private:
		Compute::Program& m_program;
		Reef::ComputeProgramTemplate m_programTemplate;
	};

    void Engine::Run() const {
        Input::Setup();

		// Asset::Importer("assets/DamagedHelmet/DamagedHelmet.gltf").Import();
		// Asset::Importer("E:/main_sponza/NewSponza_Main_glTF_003.gltf").Import();
		// Asset::Importer("E:/JungleRuins/gltf/JungleRuins_Main.gltf").Import();
		// Asset::Importer("E:/pkg_e_knight_anim/Exports/alembic/knight_ANIM_001.rnd.abc").Import();

  //   	auto colorImage = Memory::Image::Builder()
  //   		.Extent(Math::Vector3u {1280u, 720u, 1u})
		// 	.Format(vk::Format::eR8G8B8A8Unorm)
		// 	.UsageFlags(vk::ImageUsageFlagBits::eStorage)
  //   		.UsageFlags(vk::ImageUsageFlagBits::eSampled)
  //   		.UsageFlags(vk::ImageUsageFlagBits::eTransferDst)
  //   		.InitialLayout(vk::ImageLayout::eGeneral)
		// 	.Build();
	 //
		// const auto colorImageView = Memory::ImageView::Builder(*colorImage)
		// 	.Build();
	 //
  //   	const auto colorImageInfo = vk::DescriptorImageInfo()
  //   		.setImageLayout(vk::ImageLayout::eGeneral)
		// 	.setImageView(**colorImageView)
  //   		.setSampler(VK_NULL_HANDLE);
	 //
   //  	const auto depthImage = Memory::Image::Builder()
			// .Extent(Math::Vector3u { 1280u, 720u, 1u })
   //  		.Format(vk::Format::eR32Uint)
   //  		.UsageFlags(vk::ImageUsageFlagBits::eStorage)
   //  		// .UsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment)
   //  		.UsageFlags(vk::ImageUsageFlagBits::eTransferDst)
   //  		.InitialLayout(vk::ImageLayout::eGeneral)
   //  		.Build();
	  //
   //  	const auto depthImageView = Memory::ImageView::Builder(*depthImage)
			// .Build();

  //   	const auto depthImageInfo = vk::DescriptorImageInfo()
		// 	.setImageLayout(vk::ImageLayout::eGeneral)
  //   		.setImageView(**depthImageView)
  //   		.setSampler(VK_NULL_HANDLE);
	 //
  //   	struct Vertex {
		// 	alignas(16) Math::Vector3f position;
		// 	alignas(16) Math::Vector3f color;
		// };
	 //
  //   	const auto vertexBuffer = Memory::Buffer::Builder()
		// 	.InstanceCount(8u)
  //   		.InstanceSize(sizeof(Vertex))
		// 	.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
  //   		.UsageFlags(vk::BufferUsageFlagBits::eTransferDst)
		// 	.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		// 	.Build();
	 //
  //   	const auto stagingBuffer = Memory::Buffer::Builder()
		// 	.InstanceCount(8u)
		// 	.InstanceSize(sizeof(Vertex))
  //   		.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
  //   		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
  //   		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
  //   		.Build();
	 //
		// std::vector<Vertex> vertices = {
		// 	{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
		// 	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
		// 	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },
		// 	{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f } },
		// 	{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f } },
		// 	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f } },
		// 	{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f } },
		// 	{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 0.0f } },
		// };
	 //
  //   	auto mappedVertices = stagingBuffer->Map<Vertex>();
  //   	std::ranges::copy(vertices, mappedVertices.begin());
  //   	stagingBuffer->Unmap();
		// vertexBuffer->CopyBuffer(*stagingBuffer);
	 //
  //   	const auto vertexBufferInfo = vk::DescriptorBufferInfo()
		// 	.setBuffer(**vertexBuffer)
		// 	.setOffset(0)
		// 	.setRange(VK_WHOLE_SIZE);
	 //
  //   	const auto indexBuffer = Memory::Buffer::Builder()
		// 	.InstanceCount(36u)
		// 	.InstanceSize(sizeof(u32))
  //   		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
		// 	.UsageFlags(vk::BufferUsageFlagBits::eTransferDst)
		// 	.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
  //   		.Build();
	 //
  //   	const auto indexStagingBuffer = Memory::Buffer::Builder()
		// 	.InstanceCount(36u)
		// 	.InstanceSize(sizeof(u32))
		// 	.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
		// 	.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
		// 	.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
		// 	.Build();
	 //
  //   	const std::vector<u32> indices = {
		// 	0, 1, 2, 2, 3, 0,
		// 	4, 5, 6, 6, 7, 4,
		// 	0, 4, 7, 7, 3, 0,
		// 	1, 5, 6, 6, 2, 1,
		// 	3, 2, 6, 6, 7, 3,
		// 	0, 1, 5, 5, 4, 0
		// };
	 //
  //   	auto mappedIndices = indexStagingBuffer->Map<u32>();
		// std::ranges::copy(indices, mappedIndices.begin());
  //   	indexStagingBuffer->Unmap();
	 //
		// indexBuffer->CopyBuffer(*indexStagingBuffer);
	 //
  //   	const auto indexBufferInfo = vk::DescriptorBufferInfo()
		// 	.setBuffer(**indexBuffer)
		// 	.setOffset(0)
		// 	.setRange(VK_WHOLE_SIZE);
	 //
  //   	const auto cameraBufferInfo = vk::DescriptorBufferInfo()
		// 	.setBuffer(*m_sceneManager->GetLoadedScene().CameraBuffer())
		// 	.setOffset(0)
		// 	.setRange(VK_WHOLE_SIZE);
	 //
	 //
		// const auto descriptorSet = Memory::Descriptor::Set::Builder(m_scheduler->DescriptorPool(), pipeline.DescriptorSetLayout(0))
		// 	.WriteImage(0, colorImageInfo)
		// 	.WriteImage(1, depthImageInfo)
  //   		.WriteBuffer(2, vertexBufferInfo)
  //   		.WriteBuffer(3, indexBufferInfo)
  //   		.WriteBuffer(4, cameraBufferInfo)
		// 	.Build();
	 //
  //   	Reef::Container<ImageTest> imageTestContainer = Reef::MakeContainer<ImageTest>(*colorImage);


		// const Utils::PerlinNoise2D noise({ 512u, 512u }, 6);
  //   	Reef::Container<ImageTest> noiseTestContainer = Reef::MakeContainer<ImageTest>(noise.Image());

		const auto image = std::make_unique<Utils::PerlinNoise3D>(Math::Vector3u(1024u), 9);

    	auto entity = std::make_unique<ECS::Entity>("Generated Planet Mesh");
    	auto& renderTarget = entity->Add<ECS::RenderTarget>();

  		const Compute::GenerateTextureMesh generateTextureMeshProgram(
  			image->Image(),
  			Math::Vector3u(8u, 8u, 8u)
  		);

    	const auto& material = m_sceneManager->GetLoadedScene().PlanetMaterial();

    	std::vector<std::unique_ptr<Graphics::Mesh>> meshes;

    	for (u32 i = 0; i < 8; i++) {
			for (u32 j = 0; j < 8; j++) {
				for (u32 k = 0; k < 8; k++) {
					auto mesh = generateTextureMeshProgram.Execute(
						Math::Vector3u(i, j, k),
						Math::Vector3u(8u, 8u, 8u)
					);
					if (!mesh) {
						continue;
					}
					renderTarget.Add(mesh.get(), &material);
					meshes.emplace_back(std::move(mesh));
				}
			}
		}

    	m_sceneManager->GetLoadedScene().Root().AddChild(std::move(entity));

    	// return;

        while (!m_window->ShouldClose()) {
	        auto startTime = std::chrono::high_resolution_clock::now();
        	m_window->PollEvents();
        	m_window->UpdateDeltaTime();
        	m_shaderManager->Update();
        	m_sceneManager->Update(m_window->DeltaTime());
        	// pipeline.Update();

            if (!m_window->IsPaused()) {
            	if (ECS::SceneManager::Get().IsSceneLoaded())
					m_sceneManager->GetLoadedScene().Update(m_window->DeltaTime());
                m_scheduler->Update(m_window->DeltaTime());
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

        	m_shaderManager->LateUpdate();

            Input::Update();

            auto end = std::chrono::high_resolution_clock::now();
            const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - startTime).count();
            m_window->SetTitle("Coral - " + std::to_string(1000000.f / static_cast<float>(elapsed)) + "fps");
        }

        Context::Device()->waitIdle();
    }
}

