//
// Created by radue on 10/13/2024.
//

#pragma once

#include <string>
#include <unordered_map>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

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

        explicit Window(const Info&);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        GLFWwindow* operator*() const { return m_window; }

        [[nodiscard]] bool ShouldClose() const { return glfwWindowShouldClose(m_window); }
        void Close() const { glfwSetWindowShouldClose(m_window, GLFW_TRUE); }
        static void PollEvents() { glfwPollEvents(); }

        [[nodiscard]] vk::Extent2D Extent() const { return m_info.extent; }
        [[nodiscard]] static std::vector<const char*> GetRequiredExtensions() ;
        [[nodiscard]] vk::SurfaceKHR CreateSurface(const vk::Instance&) const;

        [[nodiscard]] bool IsPaused() const { return m_paused; }

        void Pause() { m_paused = true; }
        void UnPause() { m_paused = false; }

        void UpdateDeltaTime();
        [[nodiscard]] float DeltaTime() const { return static_cast<float>(m_deltaTime); }

        void SetTitle(const std::string &title) {
            m_info.title = title;
            glfwSetWindowTitle(m_window, title.c_str());
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

        GLFWwindow* m_window;
        GLFWmonitor* m_monitor;
        const GLFWvidmode *m_videoMode;

        Info m_info;
        bool m_paused = false;

        double m_lastTime = 0.0;
        double m_deltaTime = 0.0;
    };
}
