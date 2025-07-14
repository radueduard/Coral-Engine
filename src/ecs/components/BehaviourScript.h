//
// Created by radue on 6/15/2025.
//

#pragma once
#include <filesystem>
#include <mono/jit/jit.h>

#include "component.h"
#include "utils/file.h"


namespace Coral::ECS {
	class BehaviourScript final : public Component {
	public:
		explicit BehaviourScript(class Entity* entity, const std::filesystem::path& scriptPath)
			: m_scriptPath(scriptPath) {
			if (!std::filesystem::exists(scriptPath)) {
				throw std::runtime_error("Script file does not exist: " + scriptPath.string());
			}

			if (scriptPath.extension() != ".cs") {
				throw std::runtime_error("Script file must have a .cs extension: " + scriptPath.string());
			}

			if (!std::filesystem::is_regular_file(scriptPath)) {
				throw std::runtime_error("Script path is not a regular file: " + scriptPath.string());
			}

			// Load the script here, e.g., compile it or prepare it for execution
			const auto scriptContent = Utils::ReadTextFile(scriptPath);
		}

		~BehaviourScript() override = default;


	private:
		std::filesystem::path m_scriptPath;

	};
}
