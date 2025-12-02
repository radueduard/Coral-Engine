//
// Created by radue on 11/3/2024.
//

#include "manager.h"

#include <iostream>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <imnodes.h>

#include "layer.h"
#include "core/scheduler.h"
#include "core/window.h"
#include "graphics/renderPass.h"
#include "memory/descriptor/pool.h"

#include "IconsFontAwesome6.h"
#include "ecs/sceneManager.h"
#include "ecs/entity.h"

#include "gui/elements/popup.h"

static void check_vk_result(VkResult err) {
    if (err == VK_SUCCESS)
        return;

    std::cerr << "[ImGui] Error: " << vk::to_string(static_cast<vk::Result>(err)) << std::endl;

    if (err < 0)
        abort();
}



namespace Coral::Reef {
	std::string GetFontPath(const FontType type) {
		switch (type) {
		case FontType::Regular:
			return "assets/fonts/Roboto-Regular.ttf";
		case FontType::Black:
			return "assets/fonts/Roboto-Black.ttf";
		case FontType::Bold:
			return "assets/fonts/Roboto-Bold.ttf";
		case FontType::Light:
			return "assets/fonts/Roboto-Light.ttf";
		case FontType::Italic:
			return "assets/fonts/Roboto-Italic.ttf";
		case FontType::Medium:
			return "assets/fonts/Roboto-Medium.ttf";
		default:
			throw std::runtime_error("Unknown font type");
		}
	}

    Manager::Manager(const CreateInfo& createInfo)
        : m_queue(createInfo.queue), m_renderPass(createInfo.renderPass), m_frameCount(createInfo.frameCount),
          m_sampleCount(createInfo.sampleCount), m_imageFormat(createInfo.imageFormat)
    {
		static bool firstTime = true;
		if (!firstTime) {
			throw std::runtime_error("GUI Manager already created!");
		}
		firstTime = false;
		Context::m_guiManager = this;

        InitDescriptorPool();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImNodes::CreateContext();
        ImGuiIO &io = ImGui::GetIO(); (void)io;


        AddFont("assets/fonts/Roboto-Regular.ttf", 13, io.Fonts->GetGlyphRangesDefault());
        // AddFont("assets/fonts/Roboto-Regular.ttf", 32, io.Fonts->GetGlyphRangesDefault());

        // for (int baseFontSize = 8; baseFontSize <= 32; baseFontSize++) {
        //     const auto fontSize = static_cast<float>(baseFontSize);
        //     AddFont("assets/fonts/Roboto-Light.ttf", fontSize, io.Fonts->GetGlyphRangesDefault());
        //     AddFont("assets/fonts/Roboto-Medium.ttf", fontSize, io.Fonts->GetGlyphRangesDefault());
        //     AddFont("assets/fonts/Roboto-Regular.ttf", fontSize, io.Fonts->GetGlyphRangesDefault());
        //     AddFont("assets/fonts/Roboto-Bold.ttf", fontSize, io.Fonts->GetGlyphRangesDefault());
        //     AddFont("assets/fonts/Roboto-Italic.ttf", fontSize, io.Fonts->GetGlyphRangesDefault());
        //     AddFont("assets/fonts/Roboto-Black.ttf", fontSize, io.Fonts->GetGlyphRangesDefault());
        // }

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        style.AntiAliasedFill = true;

        ImGui_ImplGlfw_InitForVulkan(*Core::Window::Get(), true);
        ImGui_ImplVulkan_InitInfo init_info = {
            .Instance = Core::Runtime::Get().Instance(),
            .PhysicalDevice = *Core::Runtime::Get().PhysicalDevice(),
            .Device = *Context::Device(),
            .QueueFamily = m_queue.Family().Index(),
            .Queue = *m_queue,
            .DescriptorPool = m_descriptorPool->Handle(),
            .RenderPass = *m_renderPass,
            .MinImageCount = 2,
            .ImageCount = m_frameCount,
            .MSAASamples = static_cast<VkSampleCountFlagBits>(m_sampleCount),
            .PipelineCache = nullptr,
            .Subpass = 0,
            .Allocator = nullptr,
            .CheckVkResultFn = check_vk_result,
        };
        ImGui_ImplVulkan_Init(&init_info);

        io.FontDefault = GetFont(FontType::Regular, 13.0f);
    }

    Manager::~Manager() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImNodes::DestroyContext();
        ImGui::DestroyContext();
        m_descriptorPool.reset();
    }

    void Manager::AddLayer(Layer *layer) {
        m_layers.emplace_back(layer);
    }

    void Manager::RemoveLayer(Layer *layer) {
        std::erase(m_layers, layer);
    }

    void Manager::Update(const float deltaTime) {

    }

    void Manager::Render(const Core::CommandBuffer& commandBuffer) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    	m_frameStarted = true;

        static ImGuiDockNodeFlags dockSpaceFlags = ImGuiDockNodeFlags_NoWindowMenuButton;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

        ImGui::Begin("DockSpace Demo", nullptr, window_flags);

        ImGui::PushStyleVar(ImGuiStyleVar_TabBarBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 10.f, 10.f });
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 10.f, 10.f });
        ImGui::PushStyleVar(ImGuiStyleVar_TabBarOverlineSize, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 10.0f, 5.0f });
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 34.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.0f);

        // set selectable item spacing
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5.0f, 5.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(11.0f, 5.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 0.5f));

        ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));


        ImVec4 bgColor = ImColor(.1f, .1f, .1f, 1.f);
        ImVec4 highlightColor = ImColor(1.f, 1.f, 1.f, 1.f);
        ImVec4 dimmedHighlightColor = ImColor(1.f, 1.f, 1.f, 0.5f);
        auto buttonColor = IM_COL32(32, 32, 32, 255);
        auto buttonHoveredColor = IM_COL32(64, 64, 64, 255);

        ImGui::PushStyleColor(ImGuiCol_TitleBg, bgColor);
        ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, bgColor);
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, bgColor);
        ImGui::PushStyleColor(ImGuiCol_Tab, bgColor);
        ImGui::PushStyleColor(ImGuiCol_TabActive, bgColor);
        ImGui::PushStyleColor(ImGuiCol_TabSelected, bgColor);
        ImGui::PushStyleColor(ImGuiCol_TabHovered, bgColor);
        ImGui::PushStyleColor(ImGuiCol_TabUnfocused, bgColor);
        ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, bgColor);
        ImGui::PushStyleColor(ImGuiCol_TabSelectedOverline, highlightColor);
        ImGui::PushStyleColor(ImGuiCol_TabDimmedSelectedOverline, dimmedHighlightColor);
        ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, dimmedHighlightColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, buttonColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, buttonColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, buttonHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_Border, dimmedHighlightColor);
        ImGui::PushStyleColor(ImGuiCol_Header, buttonColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, buttonHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, dimmedHighlightColor);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, dimmedHighlightColor);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        ImGui::PushFont(GetFont(FontType::Black, 15.f));

        const ImGuiID dockSpaceId = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), dockSpaceFlags);

        ImGui::PushFont(GetFont(FontType::Regular, 13.f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

		// App menu bar
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Project")) {
					ECS::SceneManager::Get().RegisterEvent([] {
						ECS::SceneManager::Get().NewScene();
						Asset::Manager::Get().Reset();
					});
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		for (auto* layer : m_layers) {
			layer->OnGUIUpdate();
		}

        for (const auto* layer : m_layers) {
            layer->OnGUIRender();
        }

        ImGui::PopFont();
        ImGui::PopFont();

        ImGui::PopStyleColor(24);
        ImGui::PopStyleVar(16);

        ImGui::End();

        ImGui::PopStyleVar(3);

        ImGui::Render();
		m_frameStarted = false;

        ImDrawData* draw_data = ImGui::GetDrawData();
        if (const bool main_is_minimized = draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f; !main_is_minimized) {
            ImGui_ImplVulkan_RenderDrawData(draw_data, *commandBuffer);
        }

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void Manager::InitDescriptorPool() {
        m_descriptorPool = Memory::Descriptor::Pool::Builder()
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

    void Manager::AddFont(std::string path, const float size, const ImWchar* ranges) {
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

	ImFont* Manager::GetFont(const FontType type, const float size) {
		const std::string fontName = std::string(magic_enum::enum_name(type).data()) + "_" + std::to_string(size);
		for (const auto font : ImGui::GetIO().Fonts->Fonts) {
			if (strcmp(font->ConfigData->Name, fontName.c_str()) == 0) {
				return font;
			}
		}
		return ImGui::GetIO().Fonts->Fonts.back();
	}
}
