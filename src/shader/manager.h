//
// Created by radue on 2/7/2025.
//

#pragma once

#include <boost/unordered_map.hpp>
#include <boost/uuid/uuid.hpp>

#include "shader.h"
#include "gui/layer.h"

namespace Shader {
    class Manager : public GUI::Layer {
    public:

    private:
        boost::unordered_map<boost::uuids::uuid, std::unique_ptr<Core::Shader>> m_shaders;
    };
}
