//
// Created by radue on 4/11/2025.
//

#pragma once

#include <mono/metadata/assembly.h>

#include <filesystem>
#include <ranges>
#include <unordered_map>

namespace Coral::Scripting {
	class Class;

    class Assembly {
    public:
        explicit Assembly(const std::filesystem::path& path);
        ~Assembly();

        Assembly(const Assembly&) = delete;
        Assembly& operator=(const Assembly&) = delete;

        const MonoAssembly* operator*() const { return m_assembly; }

        [[nodiscard]] Class* GetClass(const std::string& namespac, const std::string& name) const;

		static const Assembly& Load(const std::filesystem::path& path) {
			auto name = path.filename().string().substr(0, path.filename().string().find_last_of('.'));
			if (s_assemblies.contains(name)) {
				return *s_assemblies[name];
			}
			return *s_assemblies.emplace(name, new Assembly(path)).first->second;
		}

		static void Unload(const std::string& name) {
			if (s_assemblies.contains(name)) {
				delete s_assemblies[name];
				s_assemblies.erase(name);
			} else {
				std::cerr << "Assembly not found: " << name << std::endl;
			}
		}

    	static void UnloadAll() {
			for (const auto assembly : s_assemblies | std::views::values) {
				delete assembly;
			}
			s_assemblies.clear();
		}

    	static const Assembly& Get(const std::string& name) {
			return Load(name);
		}

    private:
		inline static std::unordered_map<std::string, Assembly*> s_assemblies;

        MonoImage* m_image;
        MonoAssembly* m_assembly;
    };

}
