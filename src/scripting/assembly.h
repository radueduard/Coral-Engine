//
// Created by radue on 4/11/2025.
//

#pragma once

#include <mono/metadata/assembly.h>

namespace Coral::Scripting {

    class Assembly {
    public:
        explicit Assembly(const std::filesystem::path& path);
        ~Assembly();

        Assembly(const Assembly&) = delete;
        Assembly& operator=(const Assembly&) = delete;

        const MonoAssembly* operator*() const { return m_assembly; }

    private:
        MonoImage* m_image;
        MonoAssembly* m_assembly;
    };

}
