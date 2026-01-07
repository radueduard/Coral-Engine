//
// Created by radue on 2/7/2025.
//

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "utils/types.h"

namespace Coral::Shader {
	class Shader;
	class SlangCompiler;

	class Manager {
    public:
        explicit Manager();
        ~Manager() = default;

    	void Update() const;

		Coral::Shader::Shader* LoadSpirV(std::vector<u32> spirVCode, std::unordered_map<std::string, std::string> semanticMap);
		Coral::Shader::Shader* SlangShader(const std::string& module, const std::string& entryPoint) const;

    	Coral::Shader::SlangCompiler& SlangCompiler() const { return *m_slangCompiler; }

    private:
        Coral::Shader::Shader* m_selectedShader = nullptr;

    	std::unique_ptr<Coral::Shader::SlangCompiler> m_slangCompiler;
		std::vector<std::unique_ptr<Coral::Shader::Shader>> m_shaders;
    };
}
