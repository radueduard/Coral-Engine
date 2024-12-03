//
// Created by radue on 10/17/2024.
//

#pragma once

#include <string>

#include <vulkan/vulkan.hpp>
#include "device.h"

namespace Core {
    class Shader {
    public:
        Shader(const std::string &, const vk::ShaderStageFlagBits & = vk::ShaderStageFlagBits::eAllGraphics);
        ~Shader();

        const vk::ShaderModule &operator*() const { return m_shaderModule; }
        const vk::ShaderStageFlagBits &Stage() const { return m_stage; }
    private:
        std::string m_path;
        vk::ShaderStageFlagBits m_stage;
        vk::ShaderModule m_shaderModule;

        static vk::ShaderModule LoadGLSLShader(const std::string &, const vk::ShaderStageFlagBits &);
        static vk::ShaderModule LoadSpirVShader(const std::vector<uint32_t> &);
        static std::vector<uint32_t> CompileGLSLToSpirV(const std::string &, const vk::ShaderStageFlagBits &);
    };

}
