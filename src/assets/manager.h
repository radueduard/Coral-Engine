//
// Created by radue on 11/5/2024.
//

#pragma once
#include <boost/unordered/unordered_map.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>

#include "graphics/objects/material.h"
#include "graphics/objects/mesh.h"
#include "graphics/objects/texture.h"
#include "graphics/objects/textureArray.h"

namespace Asset {
    class Manager {
    public:
        class Texture {
            friend class Manager;
            static boost::uuids::uuid s_blackTextureId;
            static boost::uuids::uuid s_whiteTextureId;
            static boost::uuids::uuid s_normalTextureId;
        public:
            static const mgv::Texture& Black() { return *textures[s_blackTextureId]; }
            static const mgv::Texture& White() { return *textures[s_whiteTextureId]; }
            static const mgv::Texture& Normal() { return *textures[s_normalTextureId]; }
        };

        static boost::uuids::uuid AddMesh(std::unique_ptr<mgv::Mesh> mesh);
        static const mgv::Mesh* GetMesh(const boost::uuids::uuid& id);
        static void RemoveMesh(const boost::uuids::uuid& id);

        static boost::uuids::uuid AddMaterial(std::unique_ptr<mgv::Material> material);
        static const mgv::Material* GetMaterial(const boost::uuids::uuid& id);
        static void RemoveMaterial(const boost::uuids::uuid& id);

        static boost::uuids::uuid AddTexture(std::unique_ptr<mgv::Texture> texture);
        static const mgv::Texture* GetTexture(const boost::uuids::uuid& id);
        static void RemoveTexture(const boost::uuids::uuid& id);

        static void AddTextureArray(const std::string &key, std::unique_ptr<mgv::TextureArray> textureArray);
        static const mgv::TextureArray& GetTextureArray(const std::string& name);
        static uint32_t TextureIdInArray(const std::string& arrayName, const std::string& textureName);

        static mgv::Mesh* GetRandomMesh();

        static void Init();
        static void Destroy();

    private:
        static boost::uuids::random_generator idProvider;
        static boost::unordered_map<boost::uuids::uuid, std::unique_ptr<mgv::Mesh>> meshes;
        static boost::unordered_map<boost::uuids::uuid, std::unique_ptr<mgv::Material>> materials;

        static boost::unordered_map<boost::uuids::uuid, std::unique_ptr<mgv::Texture>> textures;
        static boost::unordered_map<std::string, std::unique_ptr<mgv::TextureArray>> textureArrays;
    };
}
