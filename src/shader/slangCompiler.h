//
// Created by Eduard Andrei Radu on 06.01.2026.
//

#pragma once

#include <slang/slang-com-ptr.h>

#include "shader.h"
#include "utils/types.h"

namespace Coral::Shader {
	class Shader;

	class EntryPoint {
	public:
		friend class Module;

		const std::string& Name() const { return m_name; }
		void LoadShader();

		Coral::Shader::Shader* Shader() const { return m_shader; }

	private:
		std::vector<u32> Compile();

		explicit EntryPoint(
			Module* module,
			u32 index,
			const Slang::ComPtr<slang::IEntryPoint>& entryPoint);

		u32 m_index;
		std::string m_name;
		std::unordered_map<std::string, std::string> m_semanticMap;

		Module* m_module;
		Slang::ComPtr<slang::IEntryPoint> m_entryPoint;

		Coral::Shader::Shader* m_shader = nullptr;
	};

	class Module {
	public:
		friend class SlangCompiler;

		const std::string& Name() const { return m_name; }
		EntryPoint& EntryPoint(const std::string& name) const;
		bool Changed() const { return m_changed; }
		slang::ISession* Session() const { return m_session; }

		void Update();

		slang::IModule* get() const { return m_module; }

	private:
		explicit Module(std::string name);

		Slang::ComPtr<slang::ISession> m_session;
		std::vector<std::filesystem::path> m_dependencies;
		std::string m_name;
		bool m_changed = false;

		u32 m_entryPointCount = 0;
		std::unordered_map<std::string, std::unique_ptr<Coral::Shader::EntryPoint>> m_entryPoints {};

		Slang::ComPtr<slang::IModule> m_module;
	};

	class SlangCompiler {
	public:
		SlangCompiler();

		bool ModuleChanged(const std::string& moduleName) const;

		void Update();

		Coral::Shader::Module& Module(const std::string& moduleName) {
			if (!modules.contains(moduleName)) {
				modules[moduleName] = LoadModule(moduleName);
			}
			return *modules.at(moduleName);
		}

		~SlangCompiler() = default;

		SlangProfileID FindProfile(const std::string& profileName) const;
		Slang::ComPtr<slang::ISession> CreateSession(const std::vector<std::filesystem::path>& searchPaths) const;

	private:
		std::unique_ptr<Coral::Shader::Module> LoadModule(const std::string& moduleName) const;

		Slang::ComPtr<slang::IGlobalSession> globalSession;

		std::unordered_map<std::string, std::unique_ptr<Coral::Shader::Module>> modules;
	};
}
