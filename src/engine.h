//
// Created by radue on 10/24/2024.
//

#pragma once
#include <memory>

#include "assets/manager.h"
#include "core/scheduler.h"
#include "ecs/sceneManager.h"
#include "shader/manager.h"


namespace Coral::Core {
    class Window;
    class Runtime;
}

namespace Coral {
	class Engine {
    public:
        Engine();
        void Run() const;

    private:
        std::unique_ptr<Core::Window> m_window;
        std::unique_ptr<Core::Runtime> m_runtime;
        std::unique_ptr<Core::Device> m_device;
		std::unique_ptr<Shader::Manager> m_shaderManager = nullptr;
        std::unique_ptr<Core::Scheduler> m_scheduler;
		std::unique_ptr<ECS::SceneManager> m_sceneManager = nullptr;
		Reef::Container<Asset::Manager> m_assetManager = nullptr;
    };
}