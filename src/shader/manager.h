//
// Created by radue on 2/7/2025.
//

#pragma once

#include "shader.h"
#include "gui/layer.h"

#include <filesystem>

#include "gui/templates/fileButton.h"
#include "gui/templates/inspector.h"

namespace Coral::Shader {
    class Manager {
    public:
        explicit Manager(std::filesystem::path defaultSearchPath);
        ~Manager() = default;

    	void Update();
    	Core::Shader* Shader(const std::filesystem::path& path) {
			if (m_shaders.contains(path)) {
				return m_shaders[path].get();
			}

			auto shader = std::make_unique<Core::Shader>(path, vk::ShaderStageFlagBits::eVertex);
			m_shaders[path] = std::move(shader);
			return m_shaders[path].get();
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

        Core::Shader* m_selectedShader = nullptr;
        std::unordered_map<std::filesystem::path, std::unique_ptr<Core::Shader>> m_shaders;
    };
}
