//
// Created by radue on 17/10/2025.
//

#pragma once

#include <boost/uuid/random_generator.hpp>


#include "utils/types.h"

namespace Coral
{
	class Scene;

	namespace Core
	{
		class Runtime;
		class Device;
		class Scheduler;
	}

	namespace Reef
	{
		class Manager;
	}

	class Context
	{
	public:
		static const Core::Runtime& Runtime() { return *m_runtime; }
		static Core::Device& Device() { return *m_device; }
		static Core::Scheduler& Scheduler() { return *m_scheduler; }
		static Reef::Manager& GUIManager() { return *m_guiManager; }
		static Coral::Scene& Scene() { return *m_scene; }

		static UUID GenerateUUID() { return m_uuidGenerator(); }
	private:
		friend class Core::Runtime;
		friend class Core::Device;
		friend class Core::Scheduler;
		friend class Reef::Manager;
		friend class Coral::Scene;

		inline static Core::Runtime* m_runtime = nullptr;
		inline static Core::Device* m_device = nullptr;
		inline static Core::Scheduler* m_scheduler = nullptr;
		inline static Reef::Manager* m_guiManager = nullptr;
		inline static Coral::Scene* m_scene = nullptr;

		inline static auto m_uuidGenerator = boost::uuids::random_generator();
	};
}
