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


namespace Coral::Reef {
	class BufferSettings;
	class ImageSettings;
	class PrefabList;
	class TextureList;
	class MaterialList;
	class MeshList;
}

namespace Coral::Asset {
	class Prefab;
	class Manager final : public Reef::Layer {
    	friend class Importer;
    public:
		Manager();
		void Reset();
		~Manager() override;

    	Manager(const Manager&) = delete;
    	Manager& operator=(const Manager&) = delete;

        void AddMesh(std::unique_ptr<Graphics::Mesh> mesh);
        const Graphics::Mesh* GetMesh(const boost::uuids::uuid& id);
        void RemoveMesh(const boost::uuids::uuid& id);

        void AddMaterial(std::unique_ptr<Graphics::Material> material);
    	const Graphics::Material* GetMaterial(const boost::uuids::uuid& id);
        void RemoveMaterial(const boost::uuids::uuid& id);

        void AddTexture(std::unique_ptr<Graphics::Texture> texture);
		bool HasTexture(const boost::uuids::uuid& id) const;
		const Graphics::Texture* GetTexture(const boost::uuids::uuid& id);
        void RemoveTexture(const boost::uuids::uuid& id);

		void AddPrefab(std::unique_ptr<Prefab> prefab);
		const Prefab& GetPrefab(const boost::uuids::uuid& id) const;
		void RemovePrefab(const boost::uuids::uuid& id);

        Graphics::Mesh* GetRandomMesh();

		static Manager& Get() {
			return *instance;
		}

	protected:
		void OnGUIAttach() override;
		void OnGUIUpdate() override;

	private:
		bool m_meshesChanged = false;
		bool m_materialsChanged = false;
		bool m_texturesChanged = false;
		bool m_prefabsChanged = false;
		void CreateUI();

		std::unique_ptr<Reef::MeshList> m_meshList;
		std::unique_ptr<Reef::MaterialList> m_materialList;
		std::unique_ptr<Reef::TextureList> m_texturesList;
		std::unique_ptr<Reef::PrefabList> m_prefabsList;

		inline static Manager* instance = nullptr;

        inline static auto idProvider = boost::uuids::random_generator();
        boost::unordered_map<boost::uuids::uuid, std::unique_ptr<Graphics::Mesh>> meshes {};
        boost::unordered_map<boost::uuids::uuid, std::unique_ptr<Graphics::Material>> materials {};
        boost::unordered_map<boost::uuids::uuid, std::unique_ptr<Graphics::Texture>> textures {};
		boost::unordered_map<boost::uuids::uuid, std::unique_ptr<Prefab>> prefabs {};
    };
}
