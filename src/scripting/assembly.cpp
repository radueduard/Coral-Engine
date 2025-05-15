//
// Created by radue on 4/11/2025.
//

#include <filesystem>
#include <iostream>

#include "assembly.h"
#include "utils/file.h"

namespace Coral::Scripting {
    Assembly::Assembly(const std::filesystem::path& path) {
        auto data = Utils::ReadTextFile(path);

        MonoImageOpenStatus status;
        m_image = mono_image_open_from_data_full(
            data.data(),
            data.size(),
            1,
            &status,
            0
        );
        if (status != MONO_IMAGE_OK) {
            const char* errorMessage = mono_image_strerror(status);
            std::cerr << "Failed to load assembly: " << errorMessage << std::endl;
            return;
        }

        m_assembly = mono_assembly_load_from_full(
            m_image,
            path.string().c_str(),
            &status,
            0
        );

        if (status != MONO_IMAGE_OK) {
            const char* errorMessage = mono_image_strerror(status);
            std::cerr << "Failed to load assembly: " << errorMessage << std::endl;
            return;
        }
    }

    Assembly::~Assembly() {
        if (m_assembly != nullptr) {
            mono_assembly_close(m_assembly);
        }
    }

}
