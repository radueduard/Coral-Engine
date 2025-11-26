//
// Created by radue on 2/7/2025.
//

#pragma once

#include "shader.h"
#include "gui/layer.h"

#include <filesystem>

#include "shader/shader.h"
#include "gui/templates/fileButton.h"
#include "gui/templates/inspector.h"

namespace Coral::Shader {
	struct Slang {
		struct Module {
			std::unordered_map<std::string, std::unique_ptr<Shader>> entryPoints;
		};

		std::unordered_map<std::string, std::unique_ptr<Module>> modules;
	};

    class Manager {
    public:
        explicit Manager(std::filesystem::path defaultSearchPath);
        ~Manager() = default;

    	void Update() const;
		void LateUpdate() const;
		//   	Core::Shader* Shader(const std::filesystem::path& path) {
		// 	if (m_shaders.contains(path)) {
		// 		return m_shaders[path].get();
		// 	}
	 //
		// 	auto shader = std::make_unique<Core::Shader>(path);
		// 	m_shaders[path] = std::move(shader);
		// 	return m_shaders[path].get();
		// }

		Shader* GetShader(const std::string& module, const std::string& entryPoint) const {
			if (!m_shaderStorage->modules.contains(module)) {
				m_shaderStorage->modules[module] = std::make_unique<Slang::Module>();
			}
			if (!m_shaderStorage->modules[module]->entryPoints.contains(entryPoint)) {
				m_shaderStorage->modules[module]->entryPoints[entryPoint] = std::make_unique<SlangShader>(module, entryPoint);
			}
			return m_shaderStorage->modules[module]->entryPoints[entryPoint].get();
		}

    	static Manager& Get() {
			if (s_instance == nullptr) {
				throw std::runtime_error("Shader Manager is not initialized");
			}
			return *s_instance;
		}

    	std::filesystem::path Path() { return m_currentPath; }

    private:
		inline static Manager* s_instance = nullptr;

        std::filesystem::path m_defaultSearchPath;
        std::filesystem::path m_currentPath;

        Shader* m_selectedShader = nullptr;
        // std::unordered_map<std::filesystem::path, std::unique_ptr<Core::Shader>> m_shaders;

    	std::unique_ptr<Slang> m_shaderStorage;
    };
}
