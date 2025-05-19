//
// Created by radue on 11/5/2024.
//

#include "manager.h"

#include "IconsFontAwesome6.h"

#include "gui/elements/popup.h"
#include "gui/elements/separator.h"

namespace Coral::Asset {
    void Manager::AddMesh(std::unique_ptr<Graphics::Mesh> mesh) {
        meshes[mesh->Id()] = std::move(mesh);
    }

    const Graphics::Mesh* Manager::GetMesh(const boost::uuids::uuid &id) {
        if (meshes.contains(id)) {
            return meshes[id].get();
        }
        return nullptr;
    }

    void Manager::RemoveMesh(const boost::uuids::uuid &id) {
        meshes.erase(id);
    }

    void Manager::AddMaterial(std::unique_ptr<Graphics::Material> material) {
        materials[material->UUID()] = std::move(material);
    }

    const Graphics::Material * Manager::GetMaterial(const boost::uuids::uuid &id) {
        if (materials.contains(id)) {
            return materials[id].get();
        }
        return nullptr;
    }

    void Manager::RemoveMaterial(const boost::uuids::uuid &id) {
        materials.erase(id);
    }

    void Manager::AddTexture(std::unique_ptr<Graphics::Texture> texture) {
        textures[texture->UUID()] = std::move(texture);
    }

    const Graphics::Texture * Manager::GetTexture(const boost::uuids::uuid &id) {
        if (textures.contains(id)) {
            return textures[id].get();
        }
        return nullptr;
    }

    void Manager::RemoveTexture(const boost::uuids::uuid &id) {
        textures.erase(id);
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

        u8 black[] = { 0, 0, 0, 255 };
        u8 white[] = { 255, 255, 255, 255 };
        u8 normal[] = { 127, 127, 255, 255 };

        auto builder = Graphics::Texture::Builder(idProvider())
            .Name("black")
            .Size(1)
            .Data(black);
        AddTexture(builder.Build());

        builder = Graphics::Texture::Builder(idProvider())
            .Name("white")
            .Size(1)
            .Data(white);
        AddTexture(builder.Build());

        builder = Graphics::Texture::Builder(idProvider())
            .Name("baseNormal")
            .Size(1)
            .Data(normal);
        AddTexture(builder.Build());

    	imageSettings = std::make_unique<Reef::ImageSettings>();
    	bufferSettings = std::make_unique<Reef::BufferSettings>();
    }

    Manager::~Manager() {
		meshes.clear();
		textures.clear();
		materials.clear();
	}

	void Manager::OnGUIAttach() {
	    Layer::OnGUIAttach();

    	AddDockable(
    		"assetManager",
    		new Reef::Dockable(
				ICON_FA_CUBE "   Asset Manager",
				{
					.padding = { 10.f, 10.f, 10.f, 10.f },
					.backgroundColor = { 0.f, 0.f, 0.f, 1.f }
				},
				{
					new Reef::Popup(
						"##CreateImage",
						{
							new Reef::Element({},
								{
									new Reef::Element(),
									new Reef::Text(
										Reef::Text::Piece(
											"Create Image",
											{
												.fontSize = 20.f,
												.fontType = Reef::FontType::Black,
											}
										),
										{
											.size = { Reef::Shrink, 20.f },
										}
									),
									new Reef::Element(),
								}
							),
							new Reef::Separator(),
							imageSettings->Build(imageBuilder),
							new Reef::Element (
								{
									.size = {Reef::Grow, Reef::Shrink },
									.spacing = 10.f,
								},
								{
									new Reef::Element(),
									new Reef::Button({
											.size = {Reef::Shrink, 23.f },
											.padding = { 10.f, 10.f, 0.f, 0.f },
											.cornerRadius = 5.f,
										},
										[this] () -> bool {
											Reef::GlobalManager().GetPopup("##CreateImage")->Close();
											imageBuilder = Memory::Image::Builder();
											return true;
										},
										{ new Reef::Text(Reef::Text::Piece("Cancel"), { .size = {Reef::Shrink, Reef::Grow } }) }
									),
									new Reef::Button(
										{
											.size = {Reef::Shrink, 23.f },
											.padding = { 10.f, 10.f, 0.f, 0.f },
											.cornerRadius = 5.f,
										},
										[this] () -> bool {
											Reef::GlobalManager().GetPopup("##CreateImage")->Close();
											auto image = imageBuilder.Build();
											imageBuilder = Memory::Image::Builder();
											return true;
										},
										{ new Reef::Text(Reef::Text::Piece("Submit"), { .size = {Reef::Shrink, Reef::Grow } }) }
									),
									new Reef::Element(),
								}
							)
						},
						{
							.padding = { 10.f, 10.f, 10.f, 10.f },
							.cornerRadius = 10.f,
							.direction = Reef::Vertical,
						}
					),
					new Reef::Popup(
						"##CreateBuffer",
						{
							new Reef::Element({},
								{
									new Reef::Element(),
									new Reef::Text(
										Reef::Text::Piece(
											"Create Buffer",
											{
												.fontSize = 20.f,
												.fontType = Reef::FontType::Black,
											}
										),
										{
											.size = { Reef::Shrink, 20.f },
										}
									),
									new Reef::Element(),
								}
							),
							new Reef::Separator(),
							bufferSettings->Build(bufferBuilder),
							new Reef::Element (
								{
									.size = {Reef::Grow, Reef::Shrink },
									.spacing = 10.f,
								},
								{
									new Reef::Element(),
									new Reef::Button({
											.size = {Reef::Shrink, 23.f },
											.padding = { 10.f, 10.f, 0.f, 0.f },
											.cornerRadius = 5.f,
										},
										[this] () -> bool {
											Reef::GlobalManager().GetPopup("##CreateBuffer")->Close();
											bufferBuilder = Memory::Buffer::Builder();
											return true;
										},
										{ new Reef::Text(Reef::Text::Piece("Cancel"), { .size = {Reef::Shrink, Reef::Grow } }) }
									),
									new Reef::Button(
										{
											.size = {Reef::Shrink, 23.f },
											.padding = { 10.f, 10.f, 0.f, 0.f },
											.cornerRadius = 5.f,
										},
										[this] () -> bool {
											Reef::GlobalManager().GetPopup("##CreateBuffer")->Close();
											auto buffer = bufferBuilder.Build();
											bufferBuilder = Memory::Buffer::Builder();
											return true;
										},
										{ new Reef::Text(Reef::Text::Piece("Submit"), { .size = {Reef::Shrink, Reef::Grow } }) }
									),
									new Reef::Element(),
								}
							)
						},
						{
							.padding = { 10.f, 10.f, 10.f, 10.f },
							.cornerRadius = 10.f,
							.direction = Reef::Vertical,
						}
					),
				},
				nullptr,
				Reef::ContextMenu::Builder()
					.AddItem("Create Image",
						[&]() -> bool {
							Reef::GlobalManager().GetPopup("##CreateImage")->Open();
							return true;
						}
					)
					.AddItem("Create Buffer",
						[&]() -> bool {
							Reef::GlobalManager().GetPopup("##CreateBuffer")->Open();
							return true;
						}
					)
					.Build()
			)
		);
    }
}
