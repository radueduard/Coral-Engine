//
// Created by radue on 10/24/2024.
//

#pragma once
#include <memory>

#include "core/scheduler.h"
#include "scripting/domain.h"

namespace Core {
    class Window;
    class Runtime;
}

class Engine {
public:
    Engine();
    ~Engine();

    void Run() const;

private:
    std::unique_ptr<Core::Window> m_window;
    std::unique_ptr<Coral::Scripting::Domain> m_scriptingDomain;
    std::unique_ptr<Core::Runtime> m_runtime;
    std::unique_ptr<Core::Device> m_device;
    std::unique_ptr<Core::Scheduler> m_scheduler;
};
