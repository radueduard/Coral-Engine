//
// Created by radue on 2/7/2025.
//

#pragma once

#include "shader.h"
#include "gui/layer.h"

#include <filesystem>

#include "gui/templates/fileButton.h"
#include "gui/templates/inspector.h"

namespace Shader {
    class Manager : public GUI::Layer {
    public:
        explicit Manager(std::filesystem::path defaultSearchPath);
        ~Manager() override = default;

        void OnGUIAttach() override;

        Core::Shader* Get(const std::filesystem::path& path);

    private:
        std::filesystem::path m_defaultSearchPath;

        std::filesystem::path m_currentPath;

        std::unique_ptr<GUI::FileButton> m_fileButtonTemplate;
        std::unique_ptr<GUI::ShaderInspector> m_shaderInspectorTemplate;

        Core::Shader* m_selectedShader = nullptr;
        std::unordered_map<std::filesystem::path, std::unique_ptr<Core::Shader>> m_shaders;
    };
}
