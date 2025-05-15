//
// Created by radue on 11/5/2024.
//

#pragma once
#include <string>
#include <boost/uuid/uuid_generators.hpp>

#include <assimp/Importer.hpp>
#include <nlohmann/json.hpp>

#include "ecs/scene.h"
#include "gui/container.h"


namespace Coral::ECS {
    class Entity;
}

namespace Coral::Asset {
    class Importer;
}

namespace Coral::Asset {
    class Importer {
    public:
        explicit Importer(const std::string& path);
        void Import();
        [[nodiscard]] Reef::Container<ECS::Scene> LoadScene() const;
    private:
        void LoadMeshes() const;
        void LoadMaterials() const;
        void LoadTextures() const;

        String m_path;
        String m_name;
        const aiScene *m_scene;
        u32 m_textureSize;
        nlohmann::json m_metadata;

        inline static boost::uuids::string_generator _stringToUuid;
        inline static Assimp::Importer _importer;
    };
}
