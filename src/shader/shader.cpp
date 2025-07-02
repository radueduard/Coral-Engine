//
// Created by radue on 10/17/2024.
//

#include "shader.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>

#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "core/device.h"
#include "utils/file.h"

#include <spirv_cross/spirv_glsl.hpp>

#include "ecs/Entity.h"
#include "graphics/objects/mesh.h"
#include "graphics/pipeline.h"
#include "gui/elements/popup.h"
#include "manager.h"

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

namespace Coral::Core {
    Shader::Shader(const std::filesystem::path &path, const vk::ShaderStageFlagBits &stage)
        : m_stage(static_cast<Core::Stage>(stage)) {
    	m_path = Coral::Shader::Manager::Get().Path() / path;
        Reload();
    }

    Shader::~Shader() {
        GlobalDevice()->destroyShaderModule(m_handle);
    }


    void Shader::LoadResourceInfo() {
        const auto module = spirv_cross::Compiler(m_spirVCode);
        const auto resources = module.get_shader_resources();

        for (const auto &sampler : resources.separate_samplers) {
            const uint32_t set = module.get_decoration(sampler.id, spv::DecorationDescriptorSet);
            const uint32_t binding = module.get_decoration(sampler.id, spv::DecorationBinding);
            const uint32_t count = module.get_type(sampler.type_id).array.size();
            m_descriptors.emplace(set, binding, vk::DescriptorType::eSampler, count);
        } // eSampler
        for (const auto &sampledImage : resources.separate_images) {
            const uint32_t set = module.get_decoration(sampledImage.id, spv::DecorationDescriptorSet);
            const uint32_t binding = module.get_decoration(sampledImage.id, spv::DecorationBinding);
            const uint32_t count = module.get_type(sampledImage.type_id).array.size();
            if (module.get_type(sampledImage.type_id).image.dim == spv::DimBuffer) {
                m_descriptors.emplace(set, binding, vk::DescriptorType::eUniformTexelBuffer, count);
            } else {
                m_descriptors.emplace(set, binding, vk::DescriptorType::eStorageImage, count);
            }
        } // eSampledImage and eUniformTexelBuffer
        for (const auto &sampledImage : resources.sampled_images) {
            const uint32_t set = module.get_decoration(sampledImage.id, spv::DecorationDescriptorSet);
            const uint32_t binding = module.get_decoration(sampledImage.id, spv::DecorationBinding);
            const uint32_t count = module.get_type(sampledImage.type_id).array.size();
            m_descriptors.emplace(set, binding, vk::DescriptorType::eCombinedImageSampler, count);
        } // eCombinedImageSampler
        for (const auto &image : resources.storage_images) {
            const uint32_t set = module.get_decoration(image.id, spv::DecorationDescriptorSet);
            const uint32_t binding = module.get_decoration(image.id, spv::DecorationBinding);
            const uint32_t count = module.get_type(image.type_id).array.size();
            if (module.get_type(image.type_id).image.dim == spv::DimBuffer) {
                m_descriptors.emplace(set, binding, vk::DescriptorType::eStorageTexelBuffer, count);
            } else {
                m_descriptors.emplace(set, binding, vk::DescriptorType::eStorageImage, count);
            }
        } // eStorageImage and eStorageTexelBuffer
        for (const auto &buffer : resources.uniform_buffers) {
            const uint32_t set = module.get_decoration(buffer.id, spv::DecorationDescriptorSet);
            const uint32_t binding = module.get_decoration(buffer.id, spv::DecorationBinding);
            const uint32_t count = module.get_type(buffer.type_id).array.size();
            m_descriptors.emplace(set, binding, vk::DescriptorType::eUniformBuffer, count);
        } // eUniformBuffer
        for (const auto &buffer : resources.storage_buffers) {
            const uint32_t set = module.get_decoration(buffer.id, spv::DecorationDescriptorSet);
            const uint32_t binding = module.get_decoration(buffer.id, spv::DecorationBinding);
            const uint32_t count = module.get_type(buffer.type_id).array.size();
            m_descriptors.emplace(set, binding, vk::DescriptorType::eStorageBuffer, count);
        } // eStorageBuffer
        for (const auto &subpassInput : resources.subpass_inputs) {
            const uint32_t set = module.get_decoration(subpassInput.id, spv::DecorationDescriptorSet);
            const uint32_t binding = module.get_decoration(subpassInput.id, spv::DecorationBinding);
            const uint32_t count = module.get_type(subpassInput.type_id).array.size();
            m_descriptors.emplace(set, binding, vk::DescriptorType::eInputAttachment, count);
        } // eInputAttachment

        for (const auto &pushConstant : resources.push_constant_buffers) {
            const uint32_t size = module.get_declared_struct_size(module.get_type(pushConstant.type_id));
            const uint32_t offset = module.get_decoration(pushConstant.id, spv::DecorationOffset);

            m_pushConstantRanges.emplace_back(size, offset);
        } // ePushConstant
    }

    nlohmann::json Shader::AnalyzeShader() const {
		auto code = Utils::ReadTextFile(m_path);
		auto analysis = nlohmann::json::object();
		analysis["name"] = m_path.filename().string();
		analysis["path"] = m_path.generic_string();
		analysis["stage"] = magic_enum::enum_name<Core::Stage>(m_stage);
		analysis["inputs"] = nlohmann::json::array();

		auto codeCopy = code;
		std::regex pragmaRegex(
			R"(#pragma\s+([a-zA-Z_]\w*)\s*layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s*in\s+([a-zA-Z_]\w*)\s+([a-zA-Z_]\w*)\s*;)");
		for (std::smatch match; std::regex_search(codeCopy, match, pragmaRegex);
			 codeCopy = codeCopy.substr(match.position() + match.length())) {
			auto attributeValues = magic_enum::enum_values<Graphics::Vertex::Attribute>();
			bool isInput = std::ranges::any_of(attributeValues, [&match](const Graphics::Vertex::Attribute& attribute) {
				return String(magic_enum::enum_name(attribute)) == match[1].str();
			});
			if (isInput) {
				auto input = nlohmann::json::object();
				input["attribute"] = match[1].str();
				input["location"] = std::stoi(match[2].str());
				input["type"] = match[3].str();
				input["name"] = match[4].str();
				analysis["inputs"].push_back(input);
			}
		}

		return analysis;
	}

	void Shader::Reload() {
    	auto extension = m_path.extension().string();
    	m_lastWriteTime = last_write_time(m_path);

    	const auto glslExtensions = std::unordered_map<std::string, vk::ShaderStageFlagBits> {
	            {".vert", vk::ShaderStageFlagBits::eVertex},
				{".tesc", vk::ShaderStageFlagBits::eTessellationControl},
				{".tese", vk::ShaderStageFlagBits::eTessellationEvaluation},
				{".geom", vk::ShaderStageFlagBits::eGeometry},
				{".frag", vk::ShaderStageFlagBits::eFragment},
				{".comp", vk::ShaderStageFlagBits::eCompute},
				{".task", vk::ShaderStageFlagBits::eTaskEXT},
				{".mesh", vk::ShaderStageFlagBits::eMeshEXT},
			};

    	if (extension == "spv") {
    		const auto pathWithoutExtension = m_path.string().substr(0, m_path.string().find_last_of('.'));
    		extension = pathWithoutExtension.substr(pathWithoutExtension.find_last_of('.') + 1);
    		if (!glslExtensions.contains(extension)) {
    			throw std::runtime_error("Unsupported shader extension: " + extension);
    		}
    		m_stage = static_cast<Core::Stage>(glslExtensions.at(extension));
    		const auto code = Utils::ReadBinaryFile(m_path);
    		m_spirVCode.resize(code.size() / sizeof(uint32_t));
    		std::memcpy(m_spirVCode.data(), code.data(), code.size());
    		m_handle = LoadSpirVShader(m_spirVCode);
    	} else if (glslExtensions.contains(extension)) {
    		m_stage = static_cast<Core::Stage>(glslExtensions.at(extension));
    		const auto code = Utils::ReadTextFile(m_path);
    		m_analysis = AnalyzeShader();
    		try {
    			m_spirVCode = CompileGLSLToSpirV(code, static_cast<vk::ShaderStageFlagBits>(m_stage));
				m_handle = LoadSpirVShader(m_spirVCode);
    			m_valid = true;
			} catch (const std::exception &e) {
				std::cerr << "Failed to compile shader: " << e.what() << std::endl;
				m_valid = false;
			}
    	} else {
    		throw std::runtime_error("Unsupported shader extension: " + extension);
    	}

    	LoadResourceInfo();
    	m_changed = true;
    }

	vk::ShaderModule Shader::LoadSpirVShader(const std::vector<uint32_t> &buffer) {
        const auto createInfo = vk::ShaderModuleCreateInfo()
            .setCode(buffer);
        return GlobalDevice()->createShaderModule(createInfo);
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

        // analyze the shader for its capabilities
        program->dumpReflection();

        delete program;
        delete shader;
        delete shaderStrings;

        glslang::FinalizeProcess();
        return spirV;
    }
}