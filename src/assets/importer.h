//
// Created by radue on 11/5/2024.
//

#pragma once
#include <string>
#include <memory>
#include <boost/uuid/uuid_generators.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <nlohmann/json.hpp>

#include "gui/container.h"

namespace Coral {
    class Object;
    class Scene;
}

namespace Asset {
    class Importer;
}

namespace Asset {
    class Importer {
    public:
        explicit Importer(const std::string& path);
        void Import();
        [[nodiscard]] GUI::Container<Coral::Scene> LoadScene() const;
    private:
        void LoadMeshes() const;
        void LoadMaterials() const;
        void LoadTextures() const;

        std::string m_path;
        std::string m_name;
        const aiScene *m_scene;
        uint32_t m_textureSize;
        nlohmann::json m_metadata;

        inline static boost::uuids::string_generator _stringToUuid;
        inline static Assimp::Importer _importer;
    };
}
