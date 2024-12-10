//
// Created by radue on 11/3/2024.
//

#include "manager.h"
#include "memory/manager.h"

#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imnodes.h"
#include "layer.h"
#include "core/runtime.h"
#include "renderer.h"
#include "core/window.h"
#include "memory/descriptor/pool.h"
#include "graphics/renderPass.h"
#include "core/physicalDevice.h"

namespace GUI {
    void Manager::Init() {
        m_instance = new Manager();
        CreateDescriptorPool();
        CreateContext();

        mgv::Renderer::InitUI();
    }

    void Manager::Destroy() {
        for (auto* layer : m_instance->m_layers) {
            layer->OnUIDetach();
        }
        mgv::Renderer::DestroyUI();

        DestroyContext();
        delete m_instance;
    }

    void Manager::AddLayer(Layer* layer) {
        m_instance->m_layersToAdd.emplace_back(layer);
    }

    void Manager::RemoveLayer(Layer* layer) {
        if (m_instance) {
            layer->OnUIReset();
            m_instance->m_layersToRemove.emplace_back(layer);
        }
    }

    void Manager::Update() {
        for (auto* layer : m_instance->m_layersToAdd) {
            m_instance->m_layers.emplace_back(layer);
            layer->OnUIAttach();
        }
        m_instance->m_layersToAdd.clear();

        for (auto* layer : m_instance->m_layersToRemove) {
            layer->OnUIDetach();
            std::erase(m_instance->m_layers, layer);
        }
        m_instance->m_layersToRemove.clear();

        for (auto* layer : m_instance->m_layers) {
            layer->OnUIUpdate();
        }

        mgv::Renderer::UpdateUI();
    }

    void Manager::Render(const vk::CommandBuffer& commandBuffer) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        if (const ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            const ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        for (auto* layer : m_instance->m_layers) {
            layer->OnUIRender();
        }
        mgv::Renderer::DrawUI();

        ImGui::End();
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        if (const bool main_is_minimized = draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f; !main_is_minimized) {
            ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
        }

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void Manager::CreateDescriptorPool() {
        m_instance->m_descriptorPool = Memory::Descriptor::Pool::Builder()
            .AddPoolSize(vk::DescriptorType::eSampler, 1000)
            .AddPoolSize(vk::DescriptorType::eCombinedImageSampler, 1000)
            .AddPoolSize(vk::DescriptorType::eSampledImage, 1000)
            .AddPoolSize(vk::DescriptorType::eStorageImage, 1000)
            .AddPoolSize(vk::DescriptorType::eUniformTexelBuffer, 1000)
            .AddPoolSize(vk::DescriptorType::eStorageTexelBuffer, 1000)
            .AddPoolSize(vk::DescriptorType::eUniformBuffer, 1000)
            .AddPoolSize(vk::DescriptorType::eStorageBuffer, 1000)
            .AddPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1000)
            .AddPoolSize(vk::DescriptorType::eStorageBufferDynamic, 1000)
            .AddPoolSize(vk::DescriptorType::eInputAttachment, 1000)
            .PoolFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .MaxSets(1000)
            .Build();
    }

    void check_vk_result(VkResult err) {
        if (err == VK_SUCCESS)
            return;

        std::cerr << "[ImGui] Error: " << vk::to_string(static_cast<vk::Result>(err)) << std::endl;

        if (err < 0)
            abort();
    }

    void Manager::CreateContext() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImNodes::CreateContext();
        ImGuiIO &io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        const auto &device = Core::Device::Get();
        const auto msaaSamples = static_cast<VkSampleCountFlagBits>(mgv::Renderer::SwapChain().SampleCount());

        ImGui_ImplGlfw_InitForVulkan(Core::Window::Get(), true);
        ImGui_ImplVulkan_InitInfo init_info = {
            .Instance = Core::Runtime::Get().Instance(),
            .PhysicalDevice = *Core::Runtime::Get().PhysicalDevice(),
            .Device = *device,
            .QueueFamily = mgv::Renderer::Queue(vk::QueueFlagBits::eGraphics).familyIndex,
            .Queue = mgv::Renderer::Queue(vk::QueueFlagBits::eGraphics).queue,
            .DescriptorPool = **m_instance->m_descriptorPool,
            .RenderPass = *mgv::Renderer::SwapChain().RenderPass(),
            .MinImageCount = mgv::Renderer::SwapChain().MinImageCount(),
            .ImageCount = mgv::Renderer::SwapChain().ImageCount(),
            .MSAASamples = msaaSamples,
            .PipelineCache = nullptr,
            .Subpass = 0,
            .Allocator = nullptr,
            .CheckVkResultFn = check_vk_result,
        };
        ImGui_ImplVulkan_Init(&init_info);
    }

    void Manager::DestroyContext() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImNodes::DestroyContext();
        ImGui::DestroyContext();
    }
}
