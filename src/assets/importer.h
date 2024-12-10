//
// Created by radue on 11/5/2024.
//

#pragma once
#include <string>
#include <memory>
#include <boost/uuid/uuid_generators.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace mgv {
    class Object;
    class Scene;
}

namespace Asset {
    class Metadata;
    class Importer;
}

namespace Asset {
    class Importer {
    public:
        explicit Importer(const std::string& path);

        void LoadChildren(const aiNode* parentNode, const std::unique_ptr<mgv::Object>& parentObject) const;

        void LoadMeshes();
        void LoadMaterials();

        void LoadBaseColorTextures() const;
        void LoadNormalTextures() const;
        void LoadMetallicRoughnessTextures() const;
        void LoadOcclusionTextures() const;
        void LoadEmissiveTextures() const;

        void LoadTextures() const;

        [[nodiscard]] std::unique_ptr<mgv::Scene> LoadScene() const;
    private:
        std::string m_path;
        std::string m_name;
        std::unique_ptr<Metadata> m_metadata;
        const aiScene *m_scene;
        uint32_t m_textureSize;

        inline static boost::uuids::string_generator _stringToUuid;
        inline static Assimp::Importer _importer;
    };
}
