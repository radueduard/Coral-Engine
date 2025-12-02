
//
// Created by radue on 11/22/2025.
//

#include "shader.h"

#include <iostream>
#include <slang/slang-com-ptr.h>

#include "context.h"
#include "core/device.h"
#include "spirv_cross.hpp"

namespace Coral::Shader {
	void Shader::PrintLayoutInfo() const {
		for (const auto& [location, name, format, semantic] : m_inputs) {
			std::cout << "layout (location = " << location << ") in "
				<< name << " : "
				<< (semantic.empty() ? " " : semantic + " : ")
				<< static_cast<u32>(format) << ";" << std::endl;
		}
		for (const auto& [location, name, format, semantic] : m_outputs) {
			std::cout << "layout (location = " << location << ") out "
				<< name << " : "
				<< (semantic.empty() ? " " : semantic + " : ")
				<< static_cast<u32>(format) << ";" << std::endl;
		}
		for (const auto& [set, binding, name, type, count] : m_descriptors) {
			std::cout << "layout (set = " << set << ", binding = " << binding << ") uniform " << static_cast<u32>(type) << " count: " << count << ";" << std::endl;
		}
		for (const auto& [size, offset, name] : m_pushConstantRanges) {
			std::cout << "push constant range size: " << size << " offset: " << offset << std::endl;
		}
	}

	Shader::~Shader() {
		if (m_handle) {
			Context::Device()->destroyShaderModule(m_handle);
		}
	}


	void Shader::LoadSpirVShader() {
		Context::Device()->waitIdle();
		if (m_handle) {
			Context::Device()->destroyShaderModule(m_handle);
		}

		const auto createInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(m_spirVCode.size() * sizeof(uint32_t))
			.setPCode(m_spirVCode.data());

		m_handle = Context::Device()->createShaderModule(createInfo);
	}

	void Shader::LoadResourceInfo(std::unordered_map<std::string, std::string> semanticMap) {
		const auto module = spirv_cross::Compiler(m_spirVCode);
		const auto resources = module.get_shader_resources();
		m_stage = static_cast<Stage>(1 << static_cast<u32>(module.get_execution_model()));

		for (const auto& input : resources.stage_inputs) {
			auto location = module.get_decoration(input.id, spv::DecorationLocation);
			const auto& name = module.get_name(input.id);
			const auto& type = module.get_type(input.type_id);
			auto semantic = semanticMap.contains(name) ? semanticMap.at(name) : "";

			m_inputs.emplace(location, name, SPIRTypeToVkFormatConverter(type), semantic);
		} // inputs
		for (const auto& output : resources.stage_outputs) {
			auto location = module.get_decoration(output.id, spv::DecorationLocation);
			auto name = module.get_name(output.id);
			const auto& type = module.get_type(output.type_id);
			auto semantic = semanticMap.contains(name) ? semanticMap.at(name) : "";

			m_outputs.emplace(location, name, SPIRTypeToVkFormatConverter(type), semantic);
		} // outputs
		for (const auto& sampler : resources.separate_samplers) {
			const uint32_t set = module.get_decoration(sampler.id, spv::DecorationDescriptorSet);
			const uint32_t binding = module.get_decoration(sampler.id, spv::DecorationBinding);
			const uint32_t count = module.get_type(sampler.type_id).array.size();
			const auto& name = module.get_name(sampler.id);
			m_descriptors.emplace(set, binding, name, vk::DescriptorType::eSampler, count);
		} // eSampler
		for (const auto& sampledImage : resources.separate_images) {
			const uint32_t set = module.get_decoration(sampledImage.id, spv::DecorationDescriptorSet);
			const uint32_t binding = module.get_decoration(sampledImage.id, spv::DecorationBinding);
			const uint32_t count = module.get_type(sampledImage.type_id).array.size();
			const auto& name = module.get_name(sampledImage.id);
			if (module.get_type(sampledImage.type_id).image.dim == spv::DimBuffer) {
				m_descriptors.emplace(set, binding, name, vk::DescriptorType::eUniformTexelBuffer, count);
			}
			else {
				m_descriptors.emplace(set, binding, name, vk::DescriptorType::eStorageImage, count);
			}
		} // eSampledImage and eUniformTexelBuffer
		for (const auto& sampledImage : resources.sampled_images) {
			const uint32_t set = module.get_decoration(sampledImage.id, spv::DecorationDescriptorSet);
			const uint32_t binding = module.get_decoration(sampledImage.id, spv::DecorationBinding);
			const uint32_t count = module.get_type(sampledImage.type_id).array.size();
			const auto& name = module.get_name(sampledImage.id);
			m_descriptors.emplace(set, binding, name, vk::DescriptorType::eCombinedImageSampler, count);
		} // eCombinedImageSampler
		for (const auto& image : resources.storage_images) {
			const uint32_t set = module.get_decoration(image.id, spv::DecorationDescriptorSet);
			const uint32_t binding = module.get_decoration(image.id, spv::DecorationBinding);
			const uint32_t count = module.get_type(image.type_id).array.size();
			const auto& name = module.get_name(image.id);
			if (module.get_type(image.type_id).image.dim == spv::DimBuffer) {
				m_descriptors.emplace(set, binding, name, vk::DescriptorType::eStorageTexelBuffer, count);
			}
			else {
				m_descriptors.emplace(set, binding, name, vk::DescriptorType::eStorageImage, count);
			}
		} // eStorageImage and eStorageTexelBuffer
		for (const auto& buffer : resources.uniform_buffers) {
			const uint32_t set = module.get_decoration(buffer.id, spv::DecorationDescriptorSet);
			const uint32_t binding = module.get_decoration(buffer.id, spv::DecorationBinding);
			const uint32_t count = module.get_type(buffer.type_id).array.size();
			const auto& name = module.get_name(buffer.id);
			m_descriptors.emplace(set, binding, name, vk::DescriptorType::eUniformBuffer, count);
		} // eUniformBuffer
		for (const auto& buffer : resources.storage_buffers) {
			const uint32_t set = module.get_decoration(buffer.id, spv::DecorationDescriptorSet);
			const uint32_t binding = module.get_decoration(buffer.id, spv::DecorationBinding);
			const uint32_t count = module.get_type(buffer.type_id).array.size();
			const auto& name = module.get_name(buffer.id);
			m_descriptors.emplace(set, binding, name, vk::DescriptorType::eStorageBuffer, count);
		} // eStorageBuffer
		for (const auto& subpassInput : resources.subpass_inputs) {
			const uint32_t set = module.get_decoration(subpassInput.id, spv::DecorationDescriptorSet);
			const uint32_t binding = module.get_decoration(subpassInput.id, spv::DecorationBinding);
			const uint32_t count = module.get_type(subpassInput.type_id).array.size();
			const auto& name = module.get_name(subpassInput.id);
			m_descriptors.emplace(set, binding, name, vk::DescriptorType::eInputAttachment, count);
		} // eInputAttachment

		for (const auto& pushConstant : resources.push_constant_buffers) {
			const uint32_t size = module.get_declared_struct_size(module.get_type(pushConstant.type_id));
			const uint32_t offset = module.get_decoration(pushConstant.id, spv::DecorationOffset);
			const auto& name = module.get_name(pushConstant.id);
			m_pushConstantRanges.emplace_back(size, offset, name);
		} // ePushConstant
	}
	SlangShader::SlangShader(const std::string& module, const std::string& entryPoint) {
		m_module = module;
		m_entryPoint = entryPoint;
		Compile();
		LoadSpirVShader();
		LoadResourceInfo(m_semanticMap);
		// PrintLayoutInfo();
	}
	void SlangShader::Update() {
		const auto currentWriteTime = std::filesystem::last_write_time(m_path);
		if (currentWriteTime != m_lastWriteTime) {
			m_lastWriteTime = currentWriteTime;
			try {
				Compile();
				m_reloaded = true;
			} catch (const std::exception& e) {
				std::cerr << "Failed to recompile shader: " << e.what() << std::endl;
				return;
			}
			LoadSpirVShader();
			LoadResourceInfo(m_semanticMap);
			// PrintLayoutInfo();
		}
	}

	void SlangShader::Compile() {
		using namespace slang;
    	Slang::ComPtr<IGlobalSession> globalSession;
    	SlangGlobalSessionDesc desc = {};
    	createGlobalSession(&desc, globalSession.writeRef());

    	TargetDesc targetDesc;
    	targetDesc.format = SLANG_SPIRV;
    	targetDesc.profile = globalSession->findProfile("glsl_450");

    	const char* searchPaths[] = { "shaders/slang" };

		constexpr PreprocessorMacroDesc fancyFlag = { "ENABLE_FANCY_FEATURE", "1" };

		const SessionDesc sessionDesc {
    		.targets = &targetDesc,
    		.targetCount = 1,
			.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
    		.searchPaths = searchPaths,
    		.searchPathCount = 1,
    		.preprocessorMacros = &fancyFlag,
    		.preprocessorMacroCount = 1,
		};

    	Slang::ComPtr<ISession> session;
    	globalSession->createSession(sessionDesc, session.writeRef());

    	Slang::ComPtr<IBlob> diagnostics;
    	const auto module = Slang::ComPtr(session->loadModule(m_module.c_str(), diagnostics.writeRef()));

    	if (diagnostics) {
			std::cerr << "Diagnostics: " << static_cast<const char*>(diagnostics->getBufferPointer()) << std::endl;
			throw std::runtime_error("Failed to load Slang module");
		}

		std::vector<IComponentType*> components;
    	components.emplace_back(module);

    	int entryPointIndex = 0;
		std::unordered_map<std::string, IEntryPoint*> entryPoints;
		const auto entryPointCount = module->getDefinedEntryPointCount();
		for (int i = 0; i < entryPointCount; ++i) {
			IEntryPoint* entryPoint;
			module->getDefinedEntryPoint(i, &entryPoint);
			if (entryPoint) {
				components.emplace_back(entryPoint);
				const auto* name = entryPoint->getFunctionReflection()->getName();
				entryPoints[name] = entryPoint;
				if (m_entryPoint == name) {
					entryPointIndex = i;
				}
			} else {
				std::cerr << "Failed to get entry point at index " << i << std::endl;
			}
		}

    	Slang::ComPtr<IComponentType> program;
    	session->createCompositeComponentType(components.data(), static_cast<i64>(components.size()), program.writeRef());

    	Slang::ComPtr<IComponentType> linkedProgram;
    	Slang::ComPtr<ISlangBlob> diagnosticBlob;
    	auto result = program->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    	if (SLANG_FAILED(result)) {
    		if (diagnosticBlob) {
    			std::cerr << "Linking diagnostics: " << static_cast<const char*>(diagnosticBlob->getBufferPointer()) << std::endl;
    		}
    		throw std::runtime_error("Failed to link Slang program");
		}

		constexpr int targetIndex = 0;
    	Slang::ComPtr<IBlob> kernelBlob;
		try {
			linkedProgram->getEntryPointCode(
				entryPointIndex,
				targetIndex,
				kernelBlob.writeRef(),
				diagnostics.writeRef());
		} catch (const std::exception& e) {
			std::cerr << "Failed to get entry point code: " << e.what() << std::endl;
			throw;
		}

		IEntryPoint* entryPoint = entryPoints[m_entryPoint];

		ProgramLayout* layout = entryPoint->getLayout();
		auto entryPointLayout = layout->findEntryPointByName(m_entryPoint.c_str());

		std::stack<std::pair<std::string, VariableLayoutReflection*>> variableLayoutStack {};
		auto parameters = entryPointLayout->getParameterCount();
		for (int i = 0; i < parameters; ++i) {
			auto parameterLayout = entryPointLayout->getParameterByIndex(i);
			variableLayoutStack.emplace("", parameterLayout);
		}

		while (!variableLayoutStack.empty()) {
			auto [parentName, variableLayout] = variableLayoutStack.top();
			variableLayoutStack.pop();

			auto name = parentName.empty() ? variableLayout->getName() : parentName + "." + variableLayout->getName();
			auto semantic = variableLayout->getSemanticName();

			if (semantic != nullptr) {
				m_semanticMap[name] = semantic;
				// std::cout << "Variable: " << name << " Semantic: " << semantic << std::endl;
			}

			const auto varType = variableLayout->getTypeLayout()->getType();
			if (varType->getKind() == TypeReflection::Kind::Struct) {
				const auto fieldCount = varType->getFieldCount();
				for (int i = 0; i < fieldCount; ++i) {
					auto fieldLayout = variableLayout->getTypeLayout()->getFieldByIndex(i);
					variableLayoutStack.emplace(name, fieldLayout);
				}
			}
		}

		m_path = module->getFilePath();
		m_lastWriteTime = std::filesystem::last_write_time(m_path);

		// get the stage of the entry point

		const u32 wordCount = static_cast<u32>(kernelBlob->getBufferSize()) / sizeof(u32);
		auto dataStart = static_cast<const u32*>(kernelBlob->getBufferPointer());
		const u32* dataEnd = dataStart + wordCount;

		m_spirVCode = {
			dataStart,
			dataEnd
		};
	}
}


// //
// // Created by radue on 10/17/2024.
// //
//
// #include "shader.h"
//
// #include <fstream>
// #include <iostream>
// #include <regex>
// #include <unordered_map>
//
// #include <glslang/Public/ResourceLimits.h>
// #include <glslang/Public/ShaderLang.h>
// #include <glslang/SPIRV/GlslangToSpv.h>
// #include <nlohmann/json.hpp>
// #include <nlohmann/json_fwd.hpp>
//
// #include "core/device.h"
// #include "utils/file.h"
//
// #include <spirv_cross/spirv_glsl.hpp>
//
// #include <slang/slang.h>
// #include <slang/slang-com-ptr.h>
//
// #include "ecs/Entity.h"
// #include "graphics/objects/mesh.h"
// #include "graphics/pipeline.h"
// #include "gui/elements/popup.h"
// #include "manager.h"
//
// static EShLanguage ShaderStageToEShLanguage(const vk::ShaderStageFlagBits &stage) {
//     switch (stage) {
//         case vk::ShaderStageFlagBits::eVertex:
//             return EShLangVertex;
//         case vk::ShaderStageFlagBits::eTessellationControl:
//             return EShLangTessControl;
//         case vk::ShaderStageFlagBits::eTessellationEvaluation:
//             return EShLangTessEvaluation;
//         case vk::ShaderStageFlagBits::eGeometry:
//             return EShLangGeometry;
//         case vk::ShaderStageFlagBits::eFragment:
//             return EShLangFragment;
//         case vk::ShaderStageFlagBits::eCompute:
//             return EShLangCompute;
//         case vk::ShaderStageFlagBits::eTaskEXT:
//             return EShLangTask;
//         case vk::ShaderStageFlagBits::eMeshEXT:
//             return EShLangMesh;
//         default:
//             throw std::runtime_error("Unsupported shader stage");
//     }
// }
//
// namespace Coral::Core {
//     Shader::Shader(const std::filesystem::path &path) {
//     	m_path = Coral::Shader::Manager::Get().Path() / path;
//         Reload();
//     }
//
//     Shader::~Shader() {
//         GlobalDevice()->destroyShaderModule(m_handle);
//     }
//
//
//     void Shader::LoadResourceInfo() {
//         const auto module = spirv_cross::Compiler(m_spirVCode);
//         const auto resources = module.get_shader_resources();
//     	m_stage = static_cast<Core::Stage>(1 << static_cast<u32>(module.get_execution_model()));
//
//         for (const auto &sampler : resources.separate_samplers) {
//             const uint32_t set = module.get_decoration(sampler.id, spv::DecorationDescriptorSet);
//             const uint32_t binding = module.get_decoration(sampler.id, spv::DecorationBinding);
//             const uint32_t count = module.get_type(sampler.type_id).array.size();
//             m_descriptors.emplace(set, binding, vk::DescriptorType::eSampler, count);
//         } // eSampler
//         for (const auto &sampledImage : resources.separate_images) {
//             const uint32_t set = module.get_decoration(sampledImage.id, spv::DecorationDescriptorSet);
//             const uint32_t binding = module.get_decoration(sampledImage.id, spv::DecorationBinding);
//             const uint32_t count = module.get_type(sampledImage.type_id).array.size();
//             if (module.get_type(sampledImage.type_id).image.dim == spv::DimBuffer) {
//                 m_descriptors.emplace(set, binding, vk::DescriptorType::eUniformTexelBuffer, count);
//             } else {
//                 m_descriptors.emplace(set, binding, vk::DescriptorType::eStorageImage, count);
//             }
//         } // eSampledImage and eUniformTexelBuffer
//         for (const auto &sampledImage : resources.sampled_images) {
//             const uint32_t set = module.get_decoration(sampledImage.id, spv::DecorationDescriptorSet);
//             const uint32_t binding = module.get_decoration(sampledImage.id, spv::DecorationBinding);
//             const uint32_t count = module.get_type(sampledImage.type_id).array.size();
//             m_descriptors.emplace(set, binding, vk::DescriptorType::eCombinedImageSampler, count);
//         } // eCombinedImageSampler
//         for (const auto &image : resources.storage_images) {
//             const uint32_t set = module.get_decoration(image.id, spv::DecorationDescriptorSet);
//             const uint32_t binding = module.get_decoration(image.id, spv::DecorationBinding);
//             const uint32_t count = module.get_type(image.type_id).array.size();
//             if (module.get_type(image.type_id).image.dim == spv::DimBuffer) {
//                 m_descriptors.emplace(set, binding, vk::DescriptorType::eStorageTexelBuffer, count);
//             } else {
//                 m_descriptors.emplace(set, binding, vk::DescriptorType::eStorageImage, count);
//             }
//         } // eStorageImage and eStorageTexelBuffer
//         for (const auto &buffer : resources.uniform_buffers) {
//             const uint32_t set = module.get_decoration(buffer.id, spv::DecorationDescriptorSet);
//             const uint32_t binding = module.get_decoration(buffer.id, spv::DecorationBinding);
//             const uint32_t count = module.get_type(buffer.type_id).array.size();
//             m_descriptors.emplace(set, binding, vk::DescriptorType::eUniformBuffer, count);
//         } // eUniformBuffer
//         for (const auto &buffer : resources.storage_buffers) {
//             const uint32_t set = module.get_decoration(buffer.id, spv::DecorationDescriptorSet);
//             const uint32_t binding = module.get_decoration(buffer.id, spv::DecorationBinding);
//             const uint32_t count = module.get_type(buffer.type_id).array.size();
//             m_descriptors.emplace(set, binding, vk::DescriptorType::eStorageBuffer, count);
//         } // eStorageBuffer
//         for (const auto &subpassInput : resources.subpass_inputs) {
//             const uint32_t set = module.get_decoration(subpassInput.id, spv::DecorationDescriptorSet);
//             const uint32_t binding = module.get_decoration(subpassInput.id, spv::DecorationBinding);
//             const uint32_t count = module.get_type(subpassInput.type_id).array.size();
//             m_descriptors.emplace(set, binding, vk::DescriptorType::eInputAttachment, count);
//         } // eInputAttachment
//
//         for (const auto &pushConstant : resources.push_constant_buffers) {
//             const uint32_t size = module.get_declared_struct_size(module.get_type(pushConstant.type_id));
//             const uint32_t offset = module.get_decoration(pushConstant.id, spv::DecorationOffset);
//
//             m_pushConstantRanges.emplace_back(size, offset);
//         } // ePushConstant
//     }
//
//     nlohmann::json Shader::AnalyzeShader() const {
// 		auto code = Utils::ReadTextFile(m_path);
// 		auto analysis = nlohmann::json::object();
// 		analysis["name"] = m_path.filename().string();
// 		analysis["path"] = m_path.generic_string();
// 		analysis["stage"] = magic_enum::enum_name<Core::Stage>(m_stage);
// 		analysis["inputs"] = nlohmann::json::array();
//
// 		auto codeCopy = code;
// 		std::regex pragmaRegex(
// 			R"(#pragma\s+([a-zA-Z_]\w*)\s*layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s*in\s+([a-zA-Z_]\w*)\s+([a-zA-Z_]\w*)\s*;)");
// 		for (std::smatch match; std::regex_search(codeCopy, match, pragmaRegex);
// 			 codeCopy = codeCopy.substr(match.position() + match.length())) {
// 			auto attributeValues = magic_enum::enum_values<Graphics::Vertex::Attribute>();
// 			bool isInput = std::ranges::any_of(attributeValues, [&match](const Graphics::Vertex::Attribute& attribute) {
// 				return String(magic_enum::enum_name(attribute)) == match[1].str();
// 			});
// 			if (isInput) {
// 				auto input = nlohmann::json::object();
// 				input["attribute"] = match[1].str();
// 				input["location"] = std::stoi(match[2].str());
// 				input["type"] = match[3].str();
// 				input["name"] = match[4].str();
// 				analysis["inputs"].push_back(input);
// 			}
// 		}
//
// 		return analysis;
// 	}
//
// 	void Shader::Reload() {
//     	auto extension = m_path.extension().string();
//     	m_lastWriteTime = last_write_time(m_path);
//
//     	const auto glslExtensions = std::unordered_map<std::string, vk::ShaderStageFlagBits> {
// 	            {".vert", vk::ShaderStageFlagBits::eVertex},
// 				{".tesc", vk::ShaderStageFlagBits::eTessellationControl},
// 				{".tese", vk::ShaderStageFlagBits::eTessellationEvaluation},
// 				{".geom", vk::ShaderStageFlagBits::eGeometry},
// 				{".frag", vk::ShaderStageFlagBits::eFragment},
// 				{".comp", vk::ShaderStageFlagBits::eCompute},
// 				{".task", vk::ShaderStageFlagBits::eTaskEXT},
// 				{".mesh", vk::ShaderStageFlagBits::eMeshEXT},
// 			};
//
//     	if (extension == ".spv") {
//     		const auto pathWithoutExtension = m_path.string().substr(0, m_path.string().find_last_of('.'));
//     		extension = pathWithoutExtension.substr(pathWithoutExtension.find_last_of('.') + 1);
//     		if (!glslExtensions.contains(extension)) {
//     			throw std::runtime_error("Unsupported shader extension: " + extension);
//     		}
//     		m_stage = static_cast<Core::Stage>(glslExtensions.at(extension));
//     		const auto code = Utils::ReadBinaryFile(m_path);
//     		m_spirVCode.resize(code.size() / sizeof(uint32_t));
//     		std::memcpy(m_spirVCode.data(), code.data(), code.size());
//     		m_handle = LoadSpirVShader(m_spirVCode);
//     	} else if (extension == ".slang") {
// 			m_stage = Core::Stage::Compute; // Slang shaders are typically compute shaders
// 			try {
// 				m_spirVCode = CompileSlangToSpirV();
//     			LoadResourceInfo();
// 				m_handle = LoadSpirVShader(m_spirVCode);
// 			} catch (const std::exception &e) {
// 				std::cerr << "Failed to compile Slang shader: " << e.what() << std::endl;
// 				m_valid = false;
// 				return;
// 			}
// 	    } else if (glslExtensions.contains(extension)) {
//     		m_stage = static_cast<Core::Stage>(glslExtensions.at(extension));
//     		const auto code = Utils::ReadTextFile(m_path);
//     		m_analysis = AnalyzeShader();
//     		try {
//     			m_spirVCode = CompileGLSLToSpirV(code, static_cast<vk::ShaderStageFlagBits>(m_stage));
//     			LoadResourceInfo();
// 				m_handle = LoadSpirVShader(m_spirVCode);
//     			m_valid = true;
// 			} catch (const std::exception &e) {
// 				std::cerr << "Failed to compile shader: " << e.what() << std::endl;
// 				m_valid = false;
// 			}
//     	} else {
//     		throw std::runtime_error("Unsupported shader extension: " + extension);
//     	}
//
//     	m_changed = true;
//     }
//
// 	vk::ShaderModule Shader::LoadSpirVShader(const std::vector<uint32_t> &buffer) {
//         const auto createInfo = vk::ShaderModuleCreateInfo()
//             .setCode(buffer);
//         return GlobalDevice()->createShaderModule(createInfo);
//     }
//
// 	std::vector<uint32_t> Shader::CompileSlangToSpirV() {
//     	using namespace slang;
//
//     	Slang::ComPtr<IGlobalSession> globalSession;
//     	SlangGlobalSessionDesc desc = {};
//     	createGlobalSession(&desc, globalSession.writeRef());
//
//     	TargetDesc targetDesc;
//     	targetDesc.format = SLANG_SPIRV;
//     	targetDesc.profile = globalSession->findProfile("glsl_450");
//
//     	const char* searchPaths[] = { "shaders/slang" };
//
//     	PreprocessorMacroDesc fancyFlag = { "ENABLE_FANCY_FEATURE", "1" };
//
//     	SessionDesc sessionDesc;
//     	sessionDesc.targets = &targetDesc;
//     	sessionDesc.targetCount = 1;
//     	sessionDesc.searchPaths = searchPaths;
//     	sessionDesc.searchPathCount = 1;
//     	sessionDesc.preprocessorMacros = &fancyFlag;
//     	sessionDesc.preprocessorMacroCount = 1;
//
//     	Slang::ComPtr<ISession> session;
//     	globalSession->createSession(sessionDesc, session.writeRef());
//
//     	Slang::ComPtr<IBlob> diagnostics;
//     	const auto module = Slang::ComPtr(session->loadModule("hello_world", diagnostics.writeRef()));
//
//     	if (diagnostics) {
// 			std::cerr << "Diagnostics: " << static_cast<const char*>(diagnostics->getBufferPointer()) << std::endl;
// 			throw std::runtime_error("Failed to load Slang module");
// 		}
//
// 		std::vector<IComponentType*> components;
//     	components.emplace_back(module);
//
// 		const auto entryPointCount = module->getDefinedEntryPointCount();
// 		for (int i = 0; i < entryPointCount; ++i) {
// 			IEntryPoint* entryPoint;
// 			module->getDefinedEntryPoint(i, &entryPoint);
// 			if (entryPoint) {
// 				components.emplace_back(entryPoint);
// 			} else {
// 				std::cerr << "Failed to get entry point at index " << i << std::endl;
// 			}
// 		}
//
//     	Slang::ComPtr<IComponentType> program;
//     	session->createCompositeComponentType(components.data(), static_cast<i64>(components.size()), program.writeRef());
//
//     	ProgramLayout* layout = program->getLayout();
// 		if (const auto l = layout->toJson(diagnostics.writeRef()); l == 0) {
// 			std::cout << "Program layout: " << static_cast<const char*>(diagnostics->getBufferPointer()) << std::endl;
// 		} else {
// 			std::cerr << "Failed to create program layout" << std::endl;
// 		}
//
//     	Slang::ComPtr<IComponentType> linkedProgram;
//     	Slang::ComPtr<ISlangBlob> diagnosticBlob;
//     	program->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
//     	if (diagnosticBlob) {
// 			std::cerr << "Linking diagnostics: " << static_cast<const char*>(diagnosticBlob->getBufferPointer()) << std::endl;
// 			throw std::runtime_error("Failed to link Slang program");
// 		}
//
//     	int entryPointIndex = 0; // only one entry point
//     	int targetIndex = 0; // only one target
//     	Slang::ComPtr<IBlob> kernelBlob;
//     	linkedProgram->getEntryPointCode(
// 			entryPointIndex,
// 			targetIndex,
// 			kernelBlob.writeRef(),
// 			diagnostics.writeRef());
//
//     	if (diagnostics) {
//     		std::cerr << "Kernel diagnostics: " << static_cast<const char*>(diagnostics->getBufferPointer()) << std::endl;
//     	}
//
//     	return {
// 			static_cast<const uint32_t*>(kernelBlob->getBufferPointer()),
// 			static_cast<const uint32_t*>(kernelBlob->getBufferPointer()) + kernelBlob->getBufferSize() / sizeof(uint32_t)
// 		};
// 	}
//
// 	std::vector<uint32_t> Shader::CompileGLSLToSpirV(const std::string &source, const vk::ShaderStageFlagBits & stage) {
//         glslang::InitializeProcess();
//
//         const auto eShStage = ShaderStageToEShLanguage(stage);
//
//         const auto shader = new glslang::TShader(eShStage);
//         const auto shaderStrings = new std::string(source);
//         const auto shaderStringsPointer = shaderStrings->c_str();
//         shader->setStrings(&shaderStringsPointer, 1);
//
//         shader->setEnvInput(glslang::EShSourceGlsl, eShStage, glslang::EShClientVulkan, 130);
//         shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
//         shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
//
//         constexpr auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDefault);
//
//         if (!shader->parse(GetDefaultResources(), 100, false, messages)) {
//             std::cerr << shader->getInfoLog() << std::endl;
//             throw std::runtime_error("GLSL parsing failed for stage: " + std::to_string(eShStage));
//         }
//
//         const auto program = new glslang::TProgram;
//         program->addShader(shader);
//
//         if (!program->link(messages)) {
//             throw std::runtime_error("GLSL linking failed for stage: " + std::to_string(eShStage));
//         }
//
//         std::vector<uint32_t> spirV;
//         GlslangToSpv(*program->getIntermediate(eShStage), spirV);
//
//         // analyze the shader for its capabilities
//         program->dumpReflection();
//
//         delete program;
//         delete shader;
//         delete shaderStrings;
//
//         glslang::FinalizeProcess();
//         return spirV;
//     }
// }