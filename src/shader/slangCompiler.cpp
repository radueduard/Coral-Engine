//
// Created by Eduard Andrei Radu on 06.01.2026.
//

#include "slangCompiler.h"
#include "context.h"

#include <iostream>
#include <ranges>
#include <stack>

#include "manager.h"

void Coral::Shader::EntryPoint::LoadShader() {
	const auto code = Compile();
	if (!m_shader) {
		m_shader = Context::ShaderManager().LoadSpirV(std::move(code), m_semanticMap);
		return;
	}

	m_shader->LoadCode(std::move(code));
	m_shader->LoadResourceInfo(m_semanticMap);
	m_shader->m_reloaded = true;
}

std::vector<Coral::u32> Coral::Shader::EntryPoint::Compile() {
	Slang::ComPtr<slang::IBlob> diagnostics;

	const std::vector<slang::IComponentType*> components{ m_module, m_entryPoint.get() };

	Slang::ComPtr<slang::IComponentType> program;
	Context::ShaderManager().SlangCompiler().Session()->createCompositeComponentType(
		components.data(), static_cast<i64>(components.size()), program.writeRef(), diagnostics.writeRef());

	if (diagnostics) {
		std::cerr << "Diagnostics: " << static_cast<const char*>(diagnostics->getBufferPointer()) << std::endl;
		throw std::runtime_error("Failed to create composite component type");
	}

	Slang::ComPtr<slang::IComponentType> linkedProgram;
	auto result = program->link(linkedProgram.writeRef(), diagnostics.writeRef());
	if (SLANG_FAILED(result)) {
		if (diagnostics) {
			std::cerr << "Linking diagnostics: " << static_cast<const char*>(diagnostics->getBufferPointer())
					  << std::endl;
		}
		throw std::runtime_error("Failed to link Slang program");
	}

	Slang::ComPtr<slang::IBlob> kernelBlob;
	try {
		constexpr int targetIndex = 0;
		result = linkedProgram->getEntryPointCode(0, targetIndex, kernelBlob.writeRef(), diagnostics.writeRef());
		if (SLANG_FAILED(result)) {
			if (diagnostics) {
				std::cerr << "Kernel diagnostics: " << static_cast<const char*>(diagnostics->getBufferPointer())
						  << std::endl;
			}
			throw std::runtime_error("Failed to get entry point code");
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to get entry point code: " << e.what() << std::endl;
		throw;
	}

	slang::ProgramLayout* layout = m_entryPoint->getLayout();
	auto entryPointLayout = layout->findEntryPointByName(m_name.c_str());

	std::stack<std::pair<std::string, slang::VariableLayoutReflection*>> variableLayoutStack{};
	const auto parameters = entryPointLayout->getParameterCount();
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

		if (const auto varType = variableLayout->getTypeLayout()->getType();
			varType->getKind() == slang::TypeReflection::Kind::Struct) {
			const auto fieldCount = varType->getFieldCount();
			for (int i = 0; i < fieldCount; ++i) {
				auto fieldLayout = variableLayout->getTypeLayout()->getFieldByIndex(i);
				variableLayoutStack.emplace(name, fieldLayout);
			}
		}
	}

	const u32 wordCount = static_cast<u32>(kernelBlob->getBufferSize()) / sizeof(u32);
	auto dataStart = static_cast<const u32*>(kernelBlob->getBufferPointer());
	const u32* dataEnd = dataStart + wordCount;

	return {dataStart, dataEnd};
}

Coral::Shader::EntryPoint::EntryPoint(slang::IModule* module, const u32 index,
									  const Slang::ComPtr<slang::IEntryPoint>& entryPoint) :
	m_index(index), m_module(module), m_entryPoint(entryPoint) {
	m_name = entryPoint->getFunctionReflection()->getName();
}

Coral::Shader::EntryPoint& Coral::Shader::Module::EntryPoint(const std::string& name) const {
	return *m_entryPoints.at(name);
}

void Coral::Shader::Module::Update() {
	m_changed = Context::FileSystemObserver().HasFileChanged(m_path);
	if (!m_changed) {
		for (const auto& dependency : m_dependencies) {
			if (Context::FileSystemObserver().HasFileChanged(dependency)) {
				m_changed = true;
				break;
			}
		}
	}
	if (m_changed) {
		std::cout << "Shader module " << m_name << " has changed. Reloading..." << std::endl;
		Context::FileSystemObserver().UntrackFile(m_path);
		for (const auto& dependency : m_dependencies) {
			Context::FileSystemObserver().UntrackFile(dependency);
		}

		m_module = Context::ShaderManager().SlangCompiler().Session()->loadModule(m_name.c_str());
		const u32 dependencyCount = m_module->getDependencyFileCount();
		m_path = m_module->getFilePath();
		Context::FileSystemObserver().TrackFile(m_path);

		m_dependencies.clear();
		m_dependencies.reserve(dependencyCount);
		for (u32 i = 0; i < dependencyCount; ++i) {
			m_dependencies.emplace_back(m_module->getDependencyFilePath(i));
			Context::FileSystemObserver().TrackFile(m_dependencies.back());
		}

		u32 newEntryPointCount = m_module->getDefinedEntryPointCount();
		std::unordered_set<std::string> newEntryPointNames;
		for (u32 i = 0; i < newEntryPointCount; ++i) {
			Slang::ComPtr<slang::IEntryPoint> entryPoint;
			m_module->getDefinedEntryPoint(i, entryPoint.writeRef());
			if (entryPoint) {
				const std::string entryPointName = entryPoint->getFunctionReflection()->getName();
				newEntryPointNames.insert(entryPointName);
				if (!m_entryPoints.contains(entryPointName)) {
					m_entryPoints.emplace(entryPointName,
						std::unique_ptr<Coral::Shader::EntryPoint>(new Coral::Shader::EntryPoint(m_module.get(), i, entryPoint)));
				} else {
					m_entryPoints[entryPointName]->m_entryPoint.swap(entryPoint);
					m_entryPoints[entryPointName]->m_index = i;
					m_entryPoints[entryPointName]->m_module = m_module.get();
				}
			}
			else {
				std::cerr << "Failed to get entry point at index " << i << std::endl;
			}
		}
		for (auto it = m_entryPoints.begin(); it != m_entryPoints.end();) {
			if (!newEntryPointNames.contains(it->first)) {
				it = m_entryPoints.erase(it);
			} else {
				++it;
			}
		}
		m_entryPointCount = newEntryPointCount;

		for (const auto& entryPoint : m_entryPoints | std::views::values) {
			entryPoint->LoadShader();
		}
	} else {
		for (const auto& entryPoint : m_entryPoints | std::views::values) {
			if (entryPoint->Shader()) {
				entryPoint->Shader()->m_reloaded = false;
			}
		}
	}
}

Coral::Shader::Module::Module(const Slang::ComPtr<slang::IModule>& module) : m_module(module) {
	m_name = m_module->getName();
	m_path = m_module->getFilePath();
	Context::FileSystemObserver().TrackFile(m_path);

	m_changed = true;
	const u32 dependencyCount = m_module->getDependencyFileCount();
	m_dependencies.reserve(dependencyCount);
	for (u32 i = 0; i < dependencyCount; ++i) {
		m_dependencies.emplace_back(m_module->getDependencyFilePath(i));
		Context::FileSystemObserver().TrackFile(m_dependencies.back());
	}
	m_entryPointCount = m_module->getDefinedEntryPointCount();
	for (u32 i = 0; i < m_entryPointCount; ++i) {
		Slang::ComPtr<slang::IEntryPoint> entryPoint;
		m_module->getDefinedEntryPoint(i, entryPoint.writeRef());
		if (entryPoint) {
			const std::string entryPointName = entryPoint->getFunctionReflection()->getName();
			m_entryPoints.emplace(entryPointName, std::unique_ptr<Coral::Shader::EntryPoint>(new Coral::Shader::EntryPoint(m_module.get(), i, entryPoint)));
		}
		else {
			std::cerr << "Failed to get entry point at index " << i << std::endl;
		}
	}
}

Coral::Shader::SlangCompiler::SlangCompiler() {
	const SlangGlobalSessionDesc desc = {};
	createGlobalSession(&desc, globalSession.writeRef());

	slang::TargetDesc targetDesc;
	targetDesc.format = SLANG_SPIRV;
	targetDesc.profile = globalSession->findProfile("spirv_1_5");

	const char* searchPaths[] = {"shaders/slang"};

	// constexpr PreprocessorMacroDesc fancyFlag = { "ENABLE_FANCY_FEATURE", "1" };

	auto slangOptions{std::to_array<slang::CompilerOptionEntry>(
		{{slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1}},
		 {slang::CompilerOptionName::PreserveParameters, {slang::CompilerOptionValueKind::Int, 1}}})};

	const slang::SessionDesc sessionDesc{
		.targets = &targetDesc,
		.targetCount = 1,
		.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
		.searchPaths = searchPaths,
		.searchPathCount = 1,
		.compilerOptionEntries = slangOptions.data(),
		.compilerOptionEntryCount = static_cast<u32>(slangOptions.size()),
		// .preprocessorMacros = &fancyFlag,
		// .preprocessorMacroCount = 1,
	};

	globalSession->createSession(sessionDesc, session.writeRef());
}

std::unique_ptr<Coral::Shader::Module> Coral::Shader::SlangCompiler::LoadModule(const std::string& moduleName) const {
	Slang::ComPtr<slang::IBlob> diagnostics;
	const auto module = Slang::ComPtr(session->loadModule(moduleName.c_str(), diagnostics.writeRef()));

	if (diagnostics) {
		std::cerr << "Diagnostics: " << static_cast<const char*>(diagnostics->getBufferPointer()) << std::endl;
	}
	return std::unique_ptr<Coral::Shader::Module>(new Coral::Shader::Module(module));
}

bool Coral::Shader::SlangCompiler::ModuleChanged(const std::string& moduleName) const {
	return modules.at(moduleName)->m_changed;
}

void Coral::Shader::SlangCompiler::Update() {
	for (const auto& module : modules | std::views::values) {
		module->Update();
	}
}
