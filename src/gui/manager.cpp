//
// Created by radue on 11/3/2024.
//

#include "manager.h"

#include <filesystem>
#include <imgui_internal.h>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imnodes.h"
#include "layer.h"
#include "core/runtime.h"
#include "core/scheduler.h"
#include "core/physicalDevice.h"
#include "core/window.h"
#include "graphics/renderPass.h"
#include "memory/descriptor/pool.h"

#include "IconsFontAwesome6.h"

namespace GUI {
    void AddLayer(Layer *layer) {
        g_layers.emplace_back(layer);
    }

    void RemoveLayer(Layer *layer) {
        std::erase(g_layers, layer);
    }

    void Render(const vk::CommandBuffer& commandBuffer) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static ImGuiDockNodeFlags dockSpaceFlags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

        if (dockSpaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        if (const ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            const ImGuiID dockSpaceId = ImGui::GetID("VulkanAppDockSpace");
            ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), dockSpaceFlags);
        }

        for (const auto* layer : g_layers) {
            layer->OnGUIRender();
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

    void InitDescriptorPool(const Core::Device& device) {
        s_descriptorPool = Memory::Descriptor::Pool::Builder()
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
            .Build(device);
    }

    void check_vk_result(VkResult err) {
        if (err == VK_SUCCESS)
            return;

        std::cerr << "[ImGui] Error: " << vk::to_string(static_cast<vk::Result>(err)) << std::endl;

        if (err < 0)
            abort();
    }

    void AddFont(std::string path, const float size, const ImWchar* ranges) {
        const std::string runPath = std::filesystem::current_path().string() + "/";
        path = runPath + path;
        const std::string iconPath = runPath + FONT_ICON_FILE_NAME_FAS;
        const float iconFontSize = size * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

        std::string name = path.substr(path.find_last_of('/') + 1);
        name = name.substr(name.find_last_of('-') + 1);
        name = name.substr(0, name.find_last_of('.'));
        name += "_" + std::to_string(size);

        ImFontConfig config;
        strcpy_s(config.Name, name.c_str());
        config.MergeMode = false;
        config.PixelSnapH = true;

        ImGui::GetIO().Fonts->AddFontFromFileTTF(path.c_str(), size, &config, ranges);
        config.MergeMode = true;
        config.GlyphMinAdvanceX = iconFontSize;
        static constexpr ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
        ImGui::GetIO().Fonts->AddFontFromFileTTF(iconPath.c_str(), iconFontSize, &config, icons_ranges);

    }

    ImFont* GetFont(const FontType type, const float size) {
        std::string fontName = std::to_string(type) + "_" + std::to_string(size);
        for (const auto font : ImGui::GetIO().Fonts->Fonts) {
            if (strcmp(font->ConfigData->Name, fontName.c_str()) == 0) {
                return font;
            }
        }
        return ImGui::GetIO().Fonts->Fonts[0];
    }

    void SetupContext(const CreateInfo& createInfo) {
        const auto& [window, runtime, device, scheduler] = createInfo;
        InitDescriptorPool(device);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImNodes::CreateContext();
        ImGuiIO &io = ImGui::GetIO(); (void)io;

        for (int baseFontSize = 8; baseFontSize <= 32; baseFontSize++) {
            const float fontSize = static_cast<float>(baseFontSize);
            AddFont("assets/fonts/Roboto-Light.ttf", fontSize, io.Fonts->GetGlyphRangesDefault());
            AddFont("assets/fonts/Roboto-Medium.ttf", fontSize, io.Fonts->GetGlyphRangesDefault());
            AddFont("assets/fonts/Roboto-Regular.ttf", fontSize, io.Fonts->GetGlyphRangesDefault());
            AddFont("assets/fonts/Roboto-Bold.ttf", fontSize, io.Fonts->GetGlyphRangesDefault());
            AddFont("assets/fonts/Roboto-Italic.ttf", fontSize, io.Fonts->GetGlyphRangesDefault());
            AddFont("assets/fonts/Roboto-Black.ttf", fontSize, io.Fonts->GetGlyphRangesDefault());
        }

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        const auto msaaSamples = static_cast<VkSampleCountFlagBits>(scheduler.SwapChain().SampleCount());

        ImGui_ImplGlfw_InitForVulkan(window.GetHandle(), true);
        ImGui_ImplVulkan_InitInfo init_info = {
            .Instance = runtime.Instance(),
            .PhysicalDevice = runtime.PhysicalDevice().Handle(),
            .Device = device.Handle(),
            .QueueFamily = scheduler.Queue(vk::QueueFlagBits::eGraphics).familyIndex,
            .Queue = scheduler.Queue(vk::QueueFlagBits::eGraphics).queue,
            .DescriptorPool = s_descriptorPool->Handle(),
            .RenderPass = scheduler.SwapChain().RenderPass().Handle(),
            .MinImageCount = scheduler.SwapChain().MinImageCount(),
            .ImageCount = scheduler.SwapChain().ImageCount(),
            .MSAASamples = msaaSamples,
            .PipelineCache = nullptr,
            .Subpass = 0,
            .Allocator = nullptr,
            .CheckVkResultFn = check_vk_result,
        };
        ImGui_ImplVulkan_Init(&init_info);

        // set default font
        io.FontDefault = GetFont(FontType::Regular, 13.0f);
    }

    void DestroyContext() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImNodes::DestroyContext();
        ImGui::DestroyContext();
        s_descriptorPool.reset();
    }
}
