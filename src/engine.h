//
// Created by radue on 10/24/2024.
//

#pragma once
#include <memory>

#include "core/scheduler.h"
#include "assets/manager.h"

namespace Core {
    class Window;
    class Runtime;
    class Device;
}

namespace Asset {
    class Manager;
}

class Engine {
public:
    Engine();
    ~Engine() = default;

    void Run() const;

private:
    std::unique_ptr<Core::Window> m_window;
    std::unique_ptr<Core::Runtime> m_runtime;
    std::unique_ptr<Core::Device> m_device;
    std::unique_ptr<Core::Scheduler> m_scheduler;
};
