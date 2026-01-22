//
// Created by radue on 2/7/2025.
//

#include "manager.h"

#include "context.h"
#include "slangCompiler.h"
#include "shader.h"

namespace Coral::Shader {
	Manager::Manager() {
		static bool firstTime = true;
		if (!firstTime) {
			throw std::runtime_error("Shader::Manager already created!");
		}
		firstTime = false;
		Context::m_shaderManager = this;

		m_slangCompiler = std::make_unique<Coral::Shader::SlangCompiler>();
	}

	void Manager::Update() const {
		m_slangCompiler->Update();
	}

	Coral::Shader::Shader* Manager::LoadSpirV(std::vector<uint32_t> spirVCode, std::unordered_map<std::string, std::string> semanticMap) {
		auto shader = std::unique_ptr<Shader>(new Shader());
		shader->LoadCode(std::move(spirVCode));
		shader->LoadResourceInfo(std::move(semanticMap));
		return m_shaders.emplace_back(std::move(shader)).get();
	}

	Coral::Shader::Shader* Manager::SlangShader(const std::string& moduleName, const std::string& entryPointName) const {
		auto& entryPoint = m_slangCompiler->Module(moduleName).EntryPoint(entryPointName);
		if (!entryPoint.Shader()) {
			entryPoint.LoadShader();
		}
		return entryPoint.Shader();
	}
}
