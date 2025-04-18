//
// Created by radue on 11/5/2024.
//

#pragma once
#include <boost/unordered/unordered_map.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>

#include "graphics/objects/mesh.h"
#include "graphics/objects/material.h"
#include "graphics/objects/texture.h"
#include "gui/layer.h"

namespace Asset {
    class Manager final : public GUI::Layer {
    public:
        static void AddMesh(std::unique_ptr<Coral::Mesh> mesh);
        static const Coral::Mesh* GetMesh(const boost::uuids::uuid& id);
        static void RemoveMesh(const boost::uuids::uuid& id);

        static void AddMaterial(std::unique_ptr<Coral::Material> material);
        static const Coral::Material* GetMaterial(const boost::uuids::uuid& id);
        static void RemoveMaterial(const boost::uuids::uuid& id);

        static void AddTexture(std::unique_ptr<Coral::Texture> texture);
        static const Coral::Texture* GetTexture(const boost::uuids::uuid& id);
        static void RemoveTexture(const boost::uuids::uuid& id);

        static Coral::Mesh* GetRandomMesh();

        static void Init();
        static void Destroy();

    private:
        inline static auto idProvider = boost::uuids::random_generator();
        inline static boost::unordered_map<boost::uuids::uuid, std::unique_ptr<Coral::Mesh>> meshes {};
        inline static boost::unordered_map<boost::uuids::uuid, std::unique_ptr<Coral::Material>> materials {};
        inline static boost::unordered_map<boost::uuids::uuid, std::unique_ptr<Coral::Texture>> textures {};
    };
}
