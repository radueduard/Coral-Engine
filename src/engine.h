//
// Created by radue on 10/24/2024.
//

#pragma once
#include <memory>

#include "core/scheduler.h"
#include "scripting/domain.h"

namespace Coral::Core {
    class Window;
    class Runtime;
}

namespace Coral {
	namespace Asset {
		class Manager;
	}

	class Engine {
    public:
        Engine();
        ~Engine();

        void Run() const;

    private:
        std::unique_ptr<Core::Window> m_window;
        std::unique_ptr<Scripting::Domain> m_scriptingDomain;
        std::unique_ptr<Core::Runtime> m_runtime;
        std::unique_ptr<Core::Device> m_device;
        std::unique_ptr<Core::Scheduler> m_scheduler;
    	std::unique_ptr<Asset::Manager> m_assetManager;
    };
}