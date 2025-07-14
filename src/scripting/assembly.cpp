//
// Created by radue on 4/11/2025.
//

#include <filesystem>
#include <iostream>
#include <vector>

#include "scripting/assembly.h"
#include "scripting/domain.h"
#include "scripting/class.h"

#include "utils/file.h"

namespace Coral::Scripting {
    Assembly::Assembly(const std::filesystem::path& path) {
        auto data = Utils::ReadTextFile(path);

        m_assembly = mono_domain_assembly_open(*Domain::Get(), path.string().c_str());
        if (m_assembly == nullptr) {
            std::cerr << "Failed to load assembly: " << path << std::endl;
            return;
        }

        m_image = mono_assembly_get_image(m_assembly);
        if (m_image == nullptr) {
            std::cerr << "Failed to get image from assembly: " << path << std::endl;
            return;
        }

        // // print all class names
        // const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(m_image, MONO_TABLE_TYPEDEF);
        // int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
        //
        // for (int32_t i = 0; i < numTypes; i++)
        // {
        //     uint32_t cols[MONO_TYPEDEF_SIZE];
        //     mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);
        //
        //     const char* nameSpace = mono_metadata_string_heap(m_image, cols[MONO_TYPEDEF_NAMESPACE]);
        //     const char* name = mono_metadata_string_heap(m_image, cols[MONO_TYPEDEF_NAME]);
        //
        //     printf("%s.%s\n", nameSpace, name);
        // }
    }

    Assembly::~Assembly() {
        if (m_assembly != nullptr) {
            mono_assembly_close(m_assembly);
        }
    }

    Class* Assembly::GetClass(const std::string& namespac, const std::string& name) const {
        return new Class(m_image, namespac, name);
    }
}
