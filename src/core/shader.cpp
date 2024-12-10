//
// Created by radue on 10/17/2024.
//

#include "shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>

#include "device.h"
#include "../utils/file.h"

static EShLanguage ShaderStageToEShLanguage(const vk::ShaderStageFlagBits &stage) {
    switch (stage) {
        case vk::ShaderStageFlagBits::eVertex:
            return EShLangVertex;
        case vk::ShaderStageFlagBits::eTessellationControl:
            return EShLangTessControl;
        case vk::ShaderStageFlagBits::eTessellationEvaluation:
            return EShLangTessEvaluation;
        case vk::ShaderStageFlagBits::eGeometry:
            return EShLangGeometry;
        case vk::ShaderStageFlagBits::eFragment:
            return EShLangFragment;
        case vk::ShaderStageFlagBits::eCompute:
            return EShLangCompute;
        case vk::ShaderStageFlagBits::eTaskEXT:
            return EShLangTask;
        case vk::ShaderStageFlagBits::eMeshEXT:
            return EShLangMesh;
        default:
            throw std::runtime_error("Unsupported shader stage");
    }
}

namespace Core {
    Shader::Shader(const std::string &path, const vk::ShaderStageFlagBits &stage) : m_path(path), m_stage(stage) {
        auto extension = path.substr(path.find_last_of('.') + 1);

        const auto glslExtensions = std::unordered_map<std::string, vk::ShaderStageFlagBits> {
            {"vert", vk::ShaderStageFlagBits::eVertex},
            {"tesc", vk::ShaderStageFlagBits::eTessellationControl},
            {"tese", vk::ShaderStageFlagBits::eTessellationEvaluation},
            {"geom", vk::ShaderStageFlagBits::eGeometry},
            {"frag", vk::ShaderStageFlagBits::eFragment},
            {"comp", vk::ShaderStageFlagBits::eCompute},
            {"task", vk::ShaderStageFlagBits::eTaskEXT},
            {"mesh", vk::ShaderStageFlagBits::eMeshEXT},
        };

        if (extension == "spv") {
            const auto pathWithoutExtension = path.substr(0, path.find_last_of('.'));
            extension = pathWithoutExtension.substr(pathWithoutExtension.find_last_of('.') + 1);
            if (!glslExtensions.contains(extension)) {
                throw std::runtime_error("Unsupported shader extension: " + extension);
            }
            m_stage = glslExtensions.at(extension);
            const auto code = Utils::ReadBinaryFile(path);
            std::vector<uint32_t> buffer(code.size() / sizeof(uint32_t));
            std::memcpy(buffer.data(), code.data(), code.size());
            m_shaderModule = LoadSpirVShader(buffer);
        } else if (glslExtensions.contains(extension)) {
            m_stage = glslExtensions.at(extension);
            const auto code = Utils::ReadTextFile(path);
            m_shaderModule = LoadGLSLShader(code, m_stage);
        } else {
            throw std::runtime_error("Unsupported shader extension: " + extension);
        }
    }

    Shader::~Shader() {
        (*Core::Device::Get()).destroyShaderModule(m_shaderModule);
    }

    vk::ShaderModule Shader::LoadGLSLShader(const std::string &code, const vk::ShaderStageFlagBits & stage) {
        const auto spirVCode = CompileGLSLToSpirV(code, stage);
        return LoadSpirVShader(spirVCode);
    }

    vk::ShaderModule Shader::LoadSpirVShader(const std::vector<uint32_t> &buffer) {
        const auto createInfo = vk::ShaderModuleCreateInfo()
            .setCode(buffer);

        return (*Device::Get()).createShaderModule(createInfo);
    }

    std::vector<uint32_t> Shader::CompileGLSLToSpirV(const std::string &source, const vk::ShaderStageFlagBits & stage) {
        glslang::InitializeProcess();

        const auto eShStage = ShaderStageToEShLanguage(stage);

        const auto shader = new glslang::TShader(eShStage);
        const auto shaderStrings = new std::string(source);
        const auto shaderStringsPointer = shaderStrings->c_str();
        shader->setStrings(&shaderStringsPointer, 1);

        shader->setEnvInput(glslang::EShSourceGlsl, eShStage, glslang::EShClientVulkan, 130);
        shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
        shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);

        constexpr auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDefault);

        if (!shader->parse(GetDefaultResources(), 100, false, messages)) {
            std::cerr << shader->getInfoLog() << std::endl;
            throw std::runtime_error("GLSL parsing failed for stage: " + std::to_string(eShStage));
        }

        const auto program = new glslang::TProgram;
        program->addShader(shader);

        if (!program->link(messages)) {
            throw std::runtime_error("GLSL linking failed for stage: " + std::to_string(eShStage));
        }

        std::vector<uint32_t> spirV;
        GlslangToSpv(*program->getIntermediate(eShStage), spirV);

        delete program;
        delete shader;
        delete shaderStrings;

        glslang::FinalizeProcess();
        return spirV;
    }
}