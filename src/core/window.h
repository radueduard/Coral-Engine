//
// Created by radue on 10/13/2024.
//

#pragma once

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

namespace Core {
    /**
     * @brief Window class
     * @details This class is used to create and manage a window using GLFW
     */
    class Window {
    public:
        /**
         * @brief Window information
         * @param title Window title
         * @param extent Window extent (width, height) in pixels as uint32_t values
         * @param resizable If the window is resizable (when fullscreen the value is ignored)
         * @param fullscreen If the window is fullscreen (overrides the extent value)
         */
        struct Info {
            std::string title;
            vk::Extent2D extent;
            bool resizable;
            bool fullscreen;
        };

        explicit Window(const Info&);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        [[nodiscard]] bool ShouldClose() const { return glfwWindowShouldClose(m_window); }
        static void PollEvents() { glfwPollEvents(); }

        [[nodiscard]] vk::Extent2D Extent() const { return m_info.extent; }
        // [[nodiscard]] GLFWwindow* Handle() const { return m_window; }
        [[nodiscard]] static std::vector<const char*> GetRequiredExtensions() ;
        [[nodiscard]] vk::SurfaceKHR CreateSurface(const vk::Instance&) const;
    private:
        /**
         * @brief Callbacks for the window
         * @details This struct contains the static methods that are used as callbacks for the window. They are set up in the constructor of the Window class
         */
        struct Callbacks {
            static void framebufferResize(GLFWwindow*, int, int);
        };

        GLFWwindow* m_window;
        GLFWmonitor* m_monitor;
        const GLFWvidmode *m_videoMode;

        Info m_info;
    }; // class Window
} // namespace Core
