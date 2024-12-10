//
// Created by radue on 11/5/2024.
//

#include "manager.h"

#include "graphics/objects/texture.h"
#include "graphics/objects/textureArray.h"

namespace Asset {
    boost::uuids::uuid Manager::Texture::s_blackTextureId;
    boost::uuids::uuid Manager::Texture::s_whiteTextureId;
    boost::uuids::uuid Manager::Texture::s_normalTextureId;

    boost::uuids::random_generator Manager::idProvider;
    boost::unordered_map<boost::uuids::uuid, std::unique_ptr<mgv::Mesh>> Manager::meshes;
    boost::unordered_map<boost::uuids::uuid, std::unique_ptr<mgv::Material>> Manager::materials;
    boost::unordered_map<boost::uuids::uuid, std::unique_ptr<mgv::Texture>> Manager::textures;
    boost::unordered_map<std::string, std::unique_ptr<mgv::TextureArray>> Manager::textureArrays;

    const mgv::Texture & Manager::Texture::Black() { return *textures[s_blackTextureId]; }

    const mgv::Texture & Manager::Texture::White() { return *textures[s_whiteTextureId]; }

    const mgv::Texture & Manager::Texture::Normal() { return *textures[s_normalTextureId]; }

    boost::uuids::uuid Manager::AddMesh(std::unique_ptr<mgv::Mesh> mesh) {
        const auto id = idProvider();
        meshes[id] = std::move(mesh);
        return id;
    }

    const mgv::Mesh * Manager::GetMesh(const boost::uuids::uuid &id) {
        if (meshes.contains(id)) {
            return meshes[id].get();
        }
        return nullptr;
    }

    void Manager::RemoveMesh(const boost::uuids::uuid &id) {
        meshes.erase(id);
    }

    boost::uuids::uuid Manager::AddMaterial(std::unique_ptr<mgv::Material> material) {
        const auto id = idProvider();
        materials[id] = std::move(material);
        return id;
    }

    const mgv::Material * Manager::GetMaterial(const boost::uuids::uuid &id) {
        if (materials.contains(id)) {
            return materials[id].get();
        }
        return nullptr;
    }

    void Manager::RemoveMaterial(const boost::uuids::uuid &id) {
        materials.erase(id);
    }

    boost::uuids::uuid Manager::AddTexture(std::unique_ptr<mgv::Texture> texture) {
        const auto id = idProvider();
        textures[id] = std::move(texture);
        return id;
    }

    const mgv::Texture * Manager::GetTexture(const boost::uuids::uuid &id) {
        if (textures.contains(id)) {
            return textures[id].get();
        }
        return nullptr;
    }

    void Manager::RemoveTexture(const boost::uuids::uuid &id) {
        textures.erase(id);
    }

    void Manager::AddTextureArray(const std::string &key, std::unique_ptr<mgv::TextureArray> textureArray) {
        textureArrays[key] = std::move(textureArray);
    }

    uint32_t Manager::TextureIdInArray(const std::string &arrayName, const std::string &textureName) {
        if (!textureArrays.contains(arrayName)) {
            return 0xff;
        }
        const mgv::TextureArray& array = *textureArrays.at(arrayName);
        return array.Id(textureName);
    }

    const mgv::TextureArray & Manager::GetTextureArray(const std::string &name) {
        return *textureArrays.at(name);
    }

    mgv::Mesh * Manager::GetRandomMesh() {
        if (meshes.empty()) {
            return nullptr;
        }
        const auto it = meshes.begin();
        return it->second.get();
    }

    void Manager::Init() {
        constexpr uint8_t black[4] {0, 0, 0, 255};
        constexpr uint8_t white[4] {255, 255, 255, 255};
        constexpr uint8_t normal[4] {127, 127, 255, 255};

        auto builder = mgv::Texture::Builder()
            .Name("black")
            .Size(1)
            .Data(black);
        const auto blackId = AddTexture(builder.Build());
        Texture::s_blackTextureId = blackId;

        builder = mgv::Texture::Builder()
            .Name("white")
            .Size(1)
            .Data(white);
        const auto whiteId = AddTexture(builder.Build());
        Texture::s_whiteTextureId = whiteId;

        builder = mgv::Texture::Builder()
            .Name("baseNormal")
            .Size(1)
            .Format(vk::Format::eR8G8B8A8Unorm)
            .Data(normal);
        const auto normalId = AddTexture(builder.Build());
        Texture::s_normalTextureId = normalId;
    }

    void Manager::Destroy() {
        meshes.clear();
        textures.clear();
        materials.clear();
        textureArrays.clear();
    }
}
