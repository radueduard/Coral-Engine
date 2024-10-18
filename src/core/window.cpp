//
// Created by radue on 10/13/2024.
//

#include "window.h"

#include <iostream>

namespace Core {
    Window::Window(const Info& createInfo) : m_info(createInfo) {
        if (const auto result = glfwInit(); result == GLFW_FALSE) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, createInfo.resizable);

        auto [width, height] = createInfo.extent;

        if (createInfo.fullscreen) {
            m_monitor = glfwGetPrimaryMonitor();
            if (m_monitor == nullptr) {
                std::cerr << "Failed to get primary monitor" << std::endl;
            }

            m_videoMode = glfwGetVideoMode(m_monitor);
            m_info.extent = vk::Extent2D {
                static_cast<unsigned int>(m_videoMode->width),
                static_cast<unsigned int>(m_videoMode->height)
            };
        } else {
            m_monitor = nullptr;
            m_videoMode = nullptr;
        }

        m_window = glfwCreateWindow(
            static_cast<int>(width),
            static_cast<int>(height),
            createInfo.title.c_str(),
            m_monitor,
            nullptr);

        if (m_window == nullptr) {
            std::cerr << "Failed to create window" << std::endl;
        }

        glfwSetWindowUserPointer(m_window, this);

        glfwSetFramebufferSizeCallback(m_window, Callbacks::framebufferResize);
    }

    Window::~Window() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    std::vector <const char*> Window::GetRequiredExtensions() {
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

    void Window::Callbacks::framebufferResize(GLFWwindow* window, const int width, const int height) {
        const auto app = static_cast<Window*>(glfwGetWindowUserPointer(window));
        app->m_info.extent = vk::Extent2D { static_cast<unsigned int>(width), static_cast<unsigned int>(height) };
        std::cout << "Resized window to " << width << "x" << height << std::endl;
    }
} // namespace Core
