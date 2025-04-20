//
// Created by radue on 4/18/2025.
//
export module core.window;

import <vulkan/vulkan.hpp>;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

import types;
import math.vector;
import input;
import std;

namespace Coral::Core {
	export class Window {
	public:
		struct CreateInfo {
			String title;
			Math::Vector2<u32> extent;
			bool resizable;
			bool fullscreen;
		};

		explicit Window(const CreateInfo& createInfo) {
			if (const auto result = glfwInit(); result == GLFW_FALSE) {
				std::cerr << "Failed to initialize GLFW" << std::endl;
			}

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, createInfo.resizable);
			// glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
			// glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

			const auto width = createInfo.extent.x;
			const auto height = createInfo.extent.y;

			if (createInfo.fullscreen) {
				m_monitor = glfwGetPrimaryMonitor();
				if (m_monitor == nullptr) {
					std::cerr << "Failed to get primary monitor" << std::endl;
				}

				m_videoMode = glfwGetVideoMode(m_monitor);
				m_extent = {
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

			glfwSetWindowUserPointer(m_window, this);

			glfwSetKeyCallback(m_window, Input::Callbacks::keyCallback);
			glfwSetCursorPosCallback(m_window, Input::Callbacks::mouseMoveCallback);
			glfwSetMouseButtonCallback(m_window, Input::Callbacks::mouseButtonCallback);
			glfwSetScrollCallback(m_window, Input::Callbacks::scrollCallback);
			glfwSetFramebufferSizeCallback(m_window, FramebufferResize);
		}
		~Window() {
			glfwDestroyWindow(m_window);
			glfwTerminate();
		}

		Window(const Window &) = delete;
		Window &operator=(const Window &) = delete;

		[[nodiscard]] bool ShouldClose() const { return glfwWindowShouldClose(m_window); }
		void Close() const { glfwSetWindowShouldClose(m_window, GLFW_TRUE); }
		void PollEvents() const { glfwPollEvents(); }

		[[nodiscard]] GLFWwindow* GetHandle() const { return m_window; }
		[[nodiscard]] Math::Vector2<u32> Extent() const { return m_extent; }

		[[nodiscard]] std::vector<const char*> GetRequiredExtensions() const {
			u32 glfwExtensionCount = 0;
			const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
			return extensions;
		}

		[[nodiscard]] vk::SurfaceKHR CreateSurface(const vk::Instance& instance) const {
			VkSurfaceKHR surface;
			if (const auto result = glfwCreateWindowSurface(instance, m_window, nullptr, &surface); result != VK_SUCCESS) {
				std::cerr << "Failed to create window surface: " << vk::to_string(static_cast<vk::Result>(result)) << std::endl;
			}

			return { surface };
		}

		[[nodiscard]] bool IsPaused() const { return m_paused; }

		void Pause() { m_paused = true; }
		void UnPause() { m_paused = false; }

		void UpdateDeltaTime() {
			const f64 currentTime = glfwGetTime();
			m_deltaTime = currentTime - m_lastTime;
			m_lastTime = currentTime;
		}

		[[nodiscard]] f64 DeltaTime() const { return m_deltaTime; }
		[[nodiscard]] f64 TimeElapsed() const { return glfwGetTime(); }

		void SetTitle(const String &title) {
			m_title = title;
			glfwSetWindowTitle(m_window, title.c_str());
		}

	private:
		static void FramebufferResize(GLFWwindow* window, i32 width, i32 height) {
			const auto app = static_cast<Window*>(glfwGetWindowUserPointer(window));

			app->m_extent = vk::Extent2D { static_cast<unsigned int>(width), static_cast<unsigned int>(height) };
			if (width == 0 || height == 0) {
				app->Pause();
			} else {
				app->UnPause();
			}
		}

		GLFWwindow* m_window;
		GLFWmonitor* m_monitor;
		const GLFWvidmode *m_videoMode;

		String m_title;
		Math::Vector2<u32> m_extent;
		bool m_resizable;
		bool m_fullscreen;
		bool m_paused = false;

		f64 m_lastTime = 0.0;
		f64 m_deltaTime = 0.0;
	};
}
