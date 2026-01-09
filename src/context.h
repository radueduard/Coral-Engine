//
// Created by radue on 17/10/2025.
//

#pragma once

#include <boost/uuid/random_generator.hpp>
#include "utils/types.h"

namespace Coral
{
	namespace Core
	{
		class Window;
		class Runtime;
		class Device;
		class Scheduler;
	}

	namespace Shader {
		class Manager;
	}

	namespace Reef
	{
		class Manager;
	}

	namespace Utils {
		class FileSystemObserver;
	}

	namespace ECS {
		class Entity;
		class Scene;
	}

	class Context
	{
	public:
		static Core::Window& Window() { return *m_window; }
		static const Core::Runtime& Runtime() { return *m_runtime; }
		static Core::Device& Device() { return *m_device; }
		static Core::Scheduler& Scheduler() { return *m_scheduler; }
		static Reef::Manager& GUIManager() { return *m_guiManager; }
		static ECS::Scene& Scene() { return *m_scene; }
		static Utils::FileSystemObserver& FileSystemObserver() { return *m_fileSystemObserver; }
		static Shader::Manager& ShaderManager() { return *m_shaderManager; }

		static UUID GenerateUUID() { return m_uuidGenerator(); }
	private:
		friend class Core::Runtime;
		friend class Core::Device;
		friend class Core::Scheduler;
		friend class Reef::Manager;
		friend class ECS::Scene;
		friend class Core::Window;
		friend class Utils::FileSystemObserver;
		friend class Shader::Manager;

		inline static Core::Window* m_window = nullptr;
		inline static Core::Runtime* m_runtime = nullptr;
		inline static Core::Device* m_device = nullptr;
		inline static Core::Scheduler* m_scheduler = nullptr;
		inline static Reef::Manager* m_guiManager = nullptr;
		inline static ECS::Scene* m_scene = nullptr;
		inline static Utils::FileSystemObserver* m_fileSystemObserver = nullptr;
		inline static Shader::Manager* m_shaderManager = nullptr;

		inline static auto m_uuidGenerator = boost::uuids::random_generator();
	};
}
