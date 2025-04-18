//
// Created by radue on 11/5/2024.
//

#include "manager.h"

#include "graphics/objects/texture.h"
#include "graphics/objects/textureArray.h"

namespace Asset {
    void Manager::AddMesh(std::unique_ptr<Coral::Mesh> mesh) {
        meshes[mesh->UUID()] = std::move(mesh);
    }

    const Coral::Mesh* Manager::GetMesh(const boost::uuids::uuid &id) {
        if (meshes.contains(id)) {
            return meshes[id].get();
        }
        return nullptr;
    }

    void Manager::RemoveMesh(const boost::uuids::uuid &id) {
        meshes.erase(id);
    }

    void Manager::AddMaterial(std::unique_ptr<Coral::Material> material) {
        materials[material->UUID()] = std::move(material);
    }

    const Coral::Material * Manager::GetMaterial(const boost::uuids::uuid &id) {
        if (materials.contains(id)) {
            return materials[id].get();
        }
        return nullptr;
    }

    void Manager::RemoveMaterial(const boost::uuids::uuid &id) {
        materials.erase(id);
    }

    void Manager::AddTexture(std::unique_ptr<Coral::Texture> texture) {
        textures[texture->UUID()] = std::move(texture);
    }

    const Coral::Texture * Manager::GetTexture(const boost::uuids::uuid &id) {
        if (textures.contains(id)) {
            return textures[id].get();
        }
        return nullptr;
    }

    void Manager::RemoveTexture(const boost::uuids::uuid &id) {
        textures.erase(id);
    }

    Coral::Mesh * Manager::GetRandomMesh() {
        if (meshes.empty()) {
            return nullptr;
        }
        const auto it = meshes.begin();
        return it->second.get();
    }

    void Manager::Init() {
        uint8_t black[] = {0, 0, 0, 255};
        uint8_t white[] = {255, 255, 255, 255};
        uint8_t normal[] = {127, 127, 255, 255};

        auto builder = Coral::Texture::Builder(idProvider())
            .Name("black")
            .Size(1)
            .Data(black);
        AddTexture(builder.Build());

        builder = Coral::Texture::Builder(idProvider())
            .Name("white")
            .Size(1)
            .Data(white);
        AddTexture(builder.Build());

        builder = Coral::Texture::Builder(idProvider())
            .Name("baseNormal")
            .Size(1)
            .Data(normal);
        AddTexture(builder.Build());
    }

    void Manager::Destroy() {
        meshes.clear();
        textures.clear();
        materials.clear();
    }
}
