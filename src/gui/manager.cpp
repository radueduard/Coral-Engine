//
// Created by radue on 11/3/2024.
//

#include "manager.h"

#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "core/runtime.h"
#include "graphics/renderer.h"

namespace GUI {
    Manager* Manager::m_instance = nullptr;

    Manager::Manager(const mgv::Renderer &renderer) : m_renderer(renderer) {
        CreateDescriptorPool();
        CreateContext();
    }

    Manager::~Manager() {
        DestroyContext();
    }

    void Manager::Init(const mgv::Renderer &renderer) {
        m_instance = new Manager(renderer);
    }

    void Manager::Destroy() {
        delete m_instance;
    }

    void Manager::AddLayer(Layer* layer) {
        m_instance->m_layers.push_back(layer);
    }

    void Manager::RemoveLayer(Layer* layer) {
        std::erase(m_instance->m_layers, layer);
    }

    void Manager::Update() {
        for (auto* layer : m_instance->m_layers) {
            layer->UpdateUI();
        }
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
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

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
            layer->DrawUI();
        }

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
        m_descriptorPool = Memory::Descriptor::Pool::Builder(m_renderer.m_device)
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

    void Manager::CreateContext() const {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
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

        const auto msaaSamples = static_cast<VkSampleCountFlagBits>(m_renderer.m_swapChainSettings.sampleCount);

        ImGui_ImplGlfw_InitForVulkan(*m_renderer.m_window, true);
        ImGui_ImplVulkan_InitInfo init_info = {
            .Instance = m_renderer.m_device.Instance(),
            .PhysicalDevice = *m_renderer.m_device.PhysicalDevice(),
            .Device = *m_renderer.m_device,
            .QueueFamily = m_renderer.m_queues.at(vk::QueueFlagBits::eGraphics)->familyIndex,
            .Queue = m_renderer.m_queues.at(vk::QueueFlagBits::eGraphics)->queue,
            .DescriptorPool = **m_descriptorPool,
            .RenderPass = *(m_renderer.m_swapChain->RenderPass()),
            .MinImageCount = m_renderer.m_swapChainSettings.minImageCount,
            .ImageCount = m_renderer.m_swapChainSettings.imageCount,
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
        ImGui::DestroyContext();
    }
}
