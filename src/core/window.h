//
// Created by radue on 10/13/2024.
//

#pragma once

#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include "math/vector.h"

namespace Coral::Core {
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
        struct CreateInfo {
            String title;
            Math::Vector2<u32> extent;
            bool resizable;
            bool fullscreen;
        };

        explicit Window(const CreateInfo&);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        [[nodiscard]] bool ShouldClose() const { return glfwWindowShouldClose(m_window); }
        void Close() const { glfwSetWindowShouldClose(m_window, GLFW_TRUE); }
        void PollEvents() const { glfwPollEvents(); }

        [[nodiscard]] GLFWwindow* GetHandle() const { return m_window; }
        [[nodiscard]] Math::Vector2<f32> Extent() const { return m_info.extent; }
        [[nodiscard]] std::vector<const char*> GetRequiredExtensions() const;
        [[nodiscard]] vk::SurfaceKHR CreateSurface(const vk::Instance&) const;

        [[nodiscard]] bool IsPaused() const { return m_paused; }

        void Pause() { m_paused = true; }
        void UnPause() { m_paused = false; }

        void UpdateDeltaTime();
        [[nodiscard]] float DeltaTime() const { return static_cast<float>(m_deltaTime); }
        [[nodiscard]] float TimeElapsed() const { return static_cast<float>(glfwGetTime()); }

        void SetTitle(const std::string &title) {
            m_info.title = title;
            glfwSetWindowTitle(m_window, title.c_str());
        }

    private:
    	static void FramebufferResize(GLFWwindow* window, int width, int height);

        GLFWwindow* m_window;
        GLFWmonitor* m_monitor;
        const GLFWvidmode *m_videoMode;

        CreateInfo m_info;
        bool m_paused = false;

        double m_lastTime = 0.0;
        double m_deltaTime = 0.0;
    };
}
