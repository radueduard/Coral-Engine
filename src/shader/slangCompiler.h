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
			slang::IModule* module,
			u32 index,
			const Slang::ComPtr<slang::IEntryPoint>& entryPoint);

		u32 m_index;
		std::string m_name;
		std::unordered_map<std::string, std::string> m_semanticMap;

		slang::IModule* m_module;
		Slang::ComPtr<slang::IEntryPoint> m_entryPoint;

		Coral::Shader::Shader* m_shader = nullptr;
	};

	class Module {
	public:
		friend class SlangCompiler;

		const std::string& Name() const { return m_name; }
		EntryPoint& EntryPoint(const std::string& name) const;

		void Update();

	private:
		explicit Module(const Slang::ComPtr<slang::IModule>& module);

		std::filesystem::path m_path;
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

		slang::ISession* Session() const { return session; }
		Coral::Shader::Module& Module(const std::string& moduleName) {
			if (!modules.contains(moduleName)) {
				modules[moduleName] = LoadModule(moduleName);
			}
			return *modules.at(moduleName);
		}

	private:
		std::unique_ptr<Coral::Shader::Module> LoadModule(const std::string& moduleName) const;

		Slang::ComPtr<slang::IGlobalSession> globalSession;
		Slang::ComPtr<slang::ISession> session;

		std::unordered_map<std::string, std::unique_ptr<Coral::Shader::Module>> modules;
	};
}
