//
// Created by radue on 11/5/2024.
//

#pragma once
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "metadata.h"
#include "core/device.h"
#include "graphics/objects/texture.h"
#include "scene/scene.h"

namespace Asset {
    class Importer {
        static boost::uuids::string_generator _stringToUuid;
        static Assimp::Importer _importer;
    public:
        explicit Importer(Core::Device &device, const std::string& path);

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
        Core::Device &m_device;
        std::string m_path;
        std::string m_name;
        Metadata m_metadata;
        const aiScene *m_scene;
        uint32_t m_textureSize;
    };
}
