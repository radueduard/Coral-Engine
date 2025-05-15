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
#include "gui/layer.h"

#include "gui/templates/imageSettings.h"
#include "gui/templates/bufferSettings.h"

namespace Coral::Asset {
    class Manager final : public Reef::Layer {
    public:
		Manager();
    	~Manager() override;

        void AddMesh(std::unique_ptr<Graphics::Mesh> mesh);
        const Graphics::Mesh* GetMesh(const boost::uuids::uuid& id);
        void RemoveMesh(const boost::uuids::uuid& id);

        void AddMaterial(std::unique_ptr<Graphics::Material> material);
    	const Graphics::Material* GetMaterial(const boost::uuids::uuid& id);
        void RemoveMaterial(const boost::uuids::uuid& id);

        void AddTexture(std::unique_ptr<Graphics::Texture> texture);
        const Graphics::Texture* GetTexture(const boost::uuids::uuid& id);
        void RemoveTexture(const boost::uuids::uuid& id);

        Graphics::Mesh* GetRandomMesh();

		static Manager& Get() {
			return *instance;
		}

	protected:
		void OnGUIAttach() override;

	private:
		inline static Manager* instance = nullptr;

        inline static auto idProvider = boost::uuids::random_generator();
        boost::unordered_map<boost::uuids::uuid, std::unique_ptr<Graphics::Mesh>> meshes {};
        boost::unordered_map<boost::uuids::uuid, std::unique_ptr<Graphics::Material>> materials {};
        boost::unordered_map<boost::uuids::uuid, std::unique_ptr<Graphics::Texture>> textures {};

    	std::unique_ptr<Reef::ImageSettings> imageSettings = nullptr;
    	Memory::Image::Builder imageBuilder {};

    	std::unique_ptr<Reef::BufferSettings> bufferSettings = nullptr;
    	Memory::Buffer::Builder bufferBuilder {};
    };
}
