//
// Created by radue on 10/24/2024.
//

#pragma once
#include <memory>

#include "core/window.h"
#include "core/runtime.h"

namespace mgv {

    class Engine {
    public:
        Engine();
        ~Engine();

        void Run() const;
    };

}
