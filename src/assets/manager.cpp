//
// Created by radue on 11/5/2024.
//

#include "manager.h"

#include "IconsFontAwesome6.h"
#include "prefab.h"

#include "gui/elements/popup.h"
#include "gui/elements/separator.h"
#include "gui/templates/importAssetPopup.h"

#include "ecs/components/RenderTarget.h"
#include "ecs/entity.h"
#include "ecs/scene.h"
#include "gui/templates/bufferSettings.h"
#include "gui/templates/imageSettings.h"
#include "gui/templates/meshList.h"

#include "graphics/objects/baseMeshes.h"
#include "gui/elements/contextMenu.h"
#include "gui/templates/createBufferPopup.h"

namespace Coral::Asset {
    void Manager::AddMesh(std::unique_ptr<Graphics::Mesh> mesh) {
        meshes[mesh->Id()] = std::move(mesh);
    	m_meshesChanged = true;
    }

    const Graphics::Mesh* Manager::GetMesh(const boost::uuids::uuid &id) {
        if (meshes.contains(id)) {
            return meshes[id].get();
        }
        return nullptr;
    }

    void Manager::RemoveMesh(const boost::uuids::uuid &id) {
        meshes.erase(id);
    	m_meshesChanged = true;
    }

    void Manager::AddMaterial(std::unique_ptr<Graphics::Material> material) {
        materials[material->UUID()] = std::move(material);
    	m_materialsChanged = true;
    }

    const Graphics::Material * Manager::GetMaterial(const boost::uuids::uuid &id) {
        if (materials.contains(id)) {
            return materials[id].get();
        }
        return nullptr;
    }

    void Manager::RemoveMaterial(const boost::uuids::uuid &id) {
        materials.erase(id);
    	m_materialsChanged = true;
    }

    void Manager::AddTexture(std::unique_ptr<Graphics::Texture> texture) {
        textures.emplace(texture->UUID(), std::move(texture));
    }

	bool Manager::HasTexture(const boost::uuids::uuid &id) const {
		return textures.contains(id);
	}

    const Graphics::Texture* Manager::GetTexture(const boost::uuids::uuid &id) {
        if (textures.contains(id)) {
            return textures[id].get();
        }
        throw std::runtime_error("Texture not found: " + boost::uuids::to_string(id));
    }

    void Manager::RemoveTexture(const boost::uuids::uuid& id) {
	    textures.erase(id);
    	m_texturesChanged = true;
    }

	void Manager::AddPrefab(std::unique_ptr<Prefab> prefab) {
		prefabs.emplace(boost::uuids::random_generator()(), std::move(prefab));
		m_prefabsChanged = true;
	}
	const Prefab& Manager::GetPrefab(const boost::uuids::uuid& id) const {
	    if (prefabs.contains(id)) {
			return *prefabs.at(id);
		}
		throw std::runtime_error("Prefab not found: " + boost::uuids::to_string(id));
	}

	void Manager::RemovePrefab(const boost::uuids::uuid& id) {
		prefabs.erase(id);
    	m_prefabsChanged = true;
    }

	Graphics::Mesh * Manager::GetRandomMesh() {
        if (meshes.empty()) {
            return nullptr;
        }
        const auto it = meshes.begin();
        return it->second.get();
    }

    Manager::Manager() {
    	instance = this;

        Reset();

    	m_meshList = std::make_unique<Reef::MeshList>();
    	m_materialList = std::make_unique<Reef::MaterialList>();
    	m_texturesList = std::make_unique<Reef::TextureList>();
    	m_prefabsList = std::make_unique<Reef::PrefabList>();
    }

	void Manager::Reset() {
		meshes.clear();
    	materials.clear();
    	textures.clear();
    	prefabs.clear();

    	std::array black {
    		Math::Vector4<u8> { static_cast<u8>(0), static_cast<u8>(0), static_cast<u8>(0), static_cast<u8>(255) },
    		Math::Vector4<u8> { static_cast<u8>(0), static_cast<u8>(0), static_cast<u8>(0), static_cast<u8>(255) },
    		Math::Vector4<u8> { static_cast<u8>(0), static_cast<u8>(0), static_cast<u8>(0), static_cast<u8>(255) },
    		Math::Vector4<u8> { static_cast<u8>(0), static_cast<u8>(0), static_cast<u8>(0), static_cast<u8>(255) },
    	};

    	std::array white {
			Math::Vector4<u8> { static_cast<u8>(255), static_cast<u8>(255), static_cast<u8>(255), static_cast<u8>(255) },
			Math::Vector4<u8> { static_cast<u8>(255), static_cast<u8>(255), static_cast<u8>(255), static_cast<u8>(255) },
			Math::Vector4<u8> { static_cast<u8>(255), static_cast<u8>(255), static_cast<u8>(255), static_cast<u8>(255) },
			Math::Vector4<u8> { static_cast<u8>(255), static_cast<u8>(255), static_cast<u8>(255), static_cast<u8>(255) },
		};

    	std::array normal {
			Math::Vector4<u8> { static_cast<u8>(127), static_cast<u8>(127), static_cast<u8>(255), static_cast<u8>(255) },
			Math::Vector4<u8> { static_cast<u8>(127), static_cast<u8>(127), static_cast<u8>(255), static_cast<u8>(255) },
			Math::Vector4<u8> { static_cast<u8>(127), static_cast<u8>(127), static_cast<u8>(255), static_cast<u8>(255) },
			Math::Vector4<u8> { static_cast<u8>(127), static_cast<u8>(127), static_cast<u8>(255), static_cast<u8>(255) },
    	};

    	auto stringGenerator = boost::uuids::string_generator();
    	auto builder = Graphics::Texture::Builder(stringGenerator("00000000-0000-0000-0000-000000000001"))
			.Name("black")
			.Size(2)
			.Data(black.data());
    	AddTexture(builder.Build());

    	builder = Graphics::Texture::Builder(stringGenerator("00000000-0000-0000-0000-000000000002"))
			.Name("white")
			.Size(2)
			.Data(white.data());
    	AddTexture(builder.Build());

    	builder = Graphics::Texture::Builder(stringGenerator("00000000-0000-0000-0000-000000000003"))
			.Name("baseNormal")
			.Size(2)
			.Data(normal.data());
    	AddTexture(builder.Build());

    	AddMesh(Graphics::Cube());
    	AddMesh(Graphics::Sphere());

    	AddMaterial(Graphics::Material::Builder(boost::uuids::nil_uuid())
			.Name("default")
			.AddTexture(PBR::Usage::Albedo, GetTexture(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000002")))
			.AddTexture(PBR::Usage::Normal, GetTexture(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000003")))
			.AddTexture(PBR::Usage::Metalic, GetTexture(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000001")))
			.AddTexture(PBR::Usage::Roughness, GetTexture(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000001")))
			.AddTexture(PBR::Usage::AmbientOcclusion, GetTexture(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000002")))
			.AddTexture(PBR::Usage::Emissive, GetTexture(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000001")))
			.Build());
    }

    Manager::~Manager() {
		meshes.clear();
		textures.clear();
		materials.clear();
	}

	void Manager::CreateUI() {
	    RemoveDockable("assetManager");

    	auto* dockable = new Reef::Window(
				ICON_FA_CUBE "   Asset Manager",
				{.padding = {10.f, 10.f, 10.f, 10.f}, .backgroundColor = {0.f, 0.f, 0.f, 1.f}},
				{
					m_meshList->Build(meshes | std::views::values |
									 std::views::transform([](const auto& pair) { return pair.get(); }) |
									 std::ranges::to<std::vector>()),
					m_materialList->Build(materials | std::views::values |
									 std::views::transform([](const auto& pair) { return pair.get(); }) |
									 std::ranges::to<std::vector>()),
					m_texturesList->Build(textures | std::views::values |
									 std::views::transform([](const auto& pair) { return pair.get(); }) |
									 std::ranges::to<std::vector>()),
					m_prefabsList->Build(prefabs | std::views::values |
									 std::views::transform([](const auto& pair) { return pair.get(); }) |
									 std::ranges::to<std::vector>()),
				},
				nullptr);

    	AddDockable("assetManager", dockable);
    }

	void Manager::OnGUIAttach() {
		Layer::OnGUIAttach();
		CreateUI();
	}

	void Manager::OnGUIUpdate() {
	    Layer::OnGUIUpdate();
    	if (m_meshesChanged || m_materialsChanged || m_texturesChanged || m_prefabsChanged) {
			CreateUI();
    		m_meshesChanged = false;
    		m_materialsChanged = false;
    		m_texturesChanged = false;
    		m_prefabsChanged = false;
		}
    }
} // namespace Coral::Asset
