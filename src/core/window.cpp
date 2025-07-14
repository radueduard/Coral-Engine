//
// Created by radue on 10/13/2024.
//

#include "window.h"
#include "input.h"

#include <iostream>
#include <mono/metadata/loader.h>
#include <stb_image.h>

auto get_elapsed() -> double {
	return Coral::Core::Window::Get().TimeElapsed();
};

auto get_deltaTime() -> double {
	return Coral::Core::Window::Get().DeltaTime();
};

auto get_fixedDeltaTime() -> double {
	return Coral::Core::Window::Get().FixedDeltaTime();
};


namespace Coral::Core {
    Window::Window(const CreateInfo& createInfo) : m_info(createInfo) {
		s_window = this;

        if (const auto result = glfwInit(); result == GLFW_FALSE) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, createInfo.resizable);
        // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);


        const u32 width = createInfo.extent.width;
        const u32 height = createInfo.extent.height;

        if (createInfo.fullscreen) {
            m_monitor = glfwGetPrimaryMonitor();
            if (m_monitor == nullptr) {
                std::cerr << "Failed to get primary monitor" << std::endl;
            }

            m_videoMode = glfwGetVideoMode(m_monitor);
            m_info.extent = Math::Vector2 {
                static_cast<u32>(m_videoMode->width),
                static_cast<u32>(m_videoMode->height)
            };
        } else {
            m_monitor = nullptr;
            m_videoMode = nullptr;
        }

        m_window = glfwCreateWindow(
            static_cast<i32>(width),
            static_cast<i32>(height),
            createInfo.title.c_str(),
            m_monitor,
            nullptr);

        if (m_window == nullptr) {
            std::cerr << "Failed to create window" << std::endl;
        }

        GLFWimage images[1];
        images[0].pixels = stbi_load("assets/icons/logo.png", &images[0].width, &images[0].height, nullptr, 4);
        if (images[0].pixels == nullptr) {
            std::cerr << "Failed to load window icon" << std::endl;
        }
        glfwSetWindowIcon(m_window, 1, images);
        stbi_image_free(images[0].pixels);

        glfwSetWindowUserPointer(m_window, this);

        glfwSetKeyCallback(m_window, Input::Callbacks::keyCallback);
        glfwSetCursorPosCallback(m_window, Input::Callbacks::mouseMoveCallback);
        glfwSetMouseButtonCallback(m_window, Input::Callbacks::mouseButtonCallback);
        glfwSetScrollCallback(m_window, Input::Callbacks::scrollCallback);
        glfwSetFramebufferSizeCallback(m_window, FramebufferResize);

    	// mono_add_internal_call("Coral.Time::get_elapsedTime", (const void*)get_elapsed);
    	// mono_add_internal_call("Coral.Time::get_deltaTime", (const void*)get_deltaTime);
    	// mono_add_internal_call("Coral.Time::get_fixedDeltaTime", (const void*)get_fixedDeltaTime);
    }

    Window::~Window() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    std::vector <const char*> Window::GetRequiredExtensions() const {
        uint32_t glfwExtensionCount = 0;
        const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        return extensions;
    }

    vk::SurfaceKHR Window::CreateSurface(const vk::Instance& instance) const {
        VkSurfaceKHR surface;
        if (const auto result = glfwCreateWindowSurface(instance, m_window, nullptr, &surface); result != VK_SUCCESS) {
            std::cerr << "Failed to create window surface: " << vk::to_string(static_cast<vk::Result>(result)) << std::endl;
        }

        return { surface };
    }

    void Window::UpdateDeltaTime() {
        const double currentTime = glfwGetTime();
        m_deltaTime = currentTime - m_lastTime;
        m_lastTime = currentTime;

    	m_timeSinceLastFixedUpdate += m_deltaTime;
    	if (m_timeSinceLastFixedUpdate >= m_fixedDeltaTime) {
			m_timeSinceLastFixedUpdate -= m_fixedDeltaTime;
    		m_timeSinceLastFixedUpdate = 0.0;
    		shouldRunFixedUpdate = true;
		}
    }

	void Window::FramebufferResize(GLFWwindow* window, const int width, const int height) {
    	const auto app = static_cast<Window*>(glfwGetWindowUserPointer(window));

    	app->m_info.extent = { static_cast<u32>(width), static_cast<u32>(height) };
    	if (width == 0 || height == 0) {
    		app->Pause();
    	} else {
    		app->UnPause();
    	}
    }

}
