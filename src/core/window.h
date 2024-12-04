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
        friend class Input;
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

        static void Init(const Info&);
        static void Destroy();

        static GLFWwindow* Get() { return m_instance->m_window; }

        explicit Window(const Info&);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        [[nodiscard]] static bool ShouldClose() { return glfwWindowShouldClose(m_instance->m_window); }
        static void Close() { glfwSetWindowShouldClose(m_instance->m_window, GLFW_TRUE); }
        static void PollEvents() { glfwPollEvents(); }

        [[nodiscard]] static vk::Extent2D Extent() { return m_instance->m_info.extent; }
        [[nodiscard]] static std::vector<const char*> GetRequiredExtensions() ;
        [[nodiscard]] static vk::SurfaceKHR CreateSurface(const vk::Instance&);

        [[nodiscard]] static bool IsPaused() { return m_instance->m_paused; }

        static void Pause() { m_instance->m_paused = true; }
        static void UnPause() { m_instance->m_paused = false; }

        static void UpdateDeltaTime();
        [[nodiscard]] static float DeltaTime() { return static_cast<float>(m_instance->m_deltaTime); }
        [[nodiscard]] static float TimeElapsed() { return static_cast<float>(glfwGetTime()); }

        static void SetTitle(const std::string &title) {
            m_instance->m_info.title = title;
            glfwSetWindowTitle(m_instance->m_window, title.c_str());
        }
    private:
        /**
         * @brief Callbacks for the window
         * @details This struct contains the static methods that are used as callbacks for the window. They are set up in the constructor of the Window class
         */
        struct Callbacks {
            static void keyCallback(GLFWwindow*, int, int, int, int);
            static void mouseMoveCallback(GLFWwindow*, double, double);
            static void mouseButtonCallback(GLFWwindow*, int, int, int);
            static void scrollCallback(GLFWwindow*, double, double);
            static void framebufferResize(GLFWwindow*, int, int);
        };

        static std::unique_ptr<Window> m_instance;

        GLFWwindow* m_window;
        GLFWmonitor* m_monitor;
        const GLFWvidmode *m_videoMode;

        Info m_info;
        bool m_paused = false;

        double m_lastTime = 0.0;
        double m_deltaTime = 0.0;
    };
}
