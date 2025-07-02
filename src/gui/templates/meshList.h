//
// Created by radue on 6/23/2025.
//

#pragma once
#include "assets/prefab.h"
#include "graphics/objects/mesh.h"
#include "gui/elements/image.h"
#include "gui/elements/text.h"
#include "template.h"

namespace Coral::Asset {
	class Prefab;
}
namespace Coral::Reef {
	class MeshList final : public ReadOnlyTemplate<std::vector<Graphics::Mesh*>> {
	public:
		Element* Build(const std::vector<Graphics::Mesh*>& meshes) override {
			std::vector<Element*> elements;
			elements.reserve(meshes.size());
			for (const auto* mesh : meshes) {
				if (mesh) {
					elements.emplace_back(new Element(
						{
							.size = {Grow, Shrink},
							.padding = {5.f, 5.f, 5.f, 5.f},
							.spacing = 5.f,
							.cornerRadius = 5.f,
						},
						{
							new Element({
								.size = {100.f, 100.f},
								.backgroundColor = {0.f, 0.f, 0.f, 1.f},
							}),
							new Text(Text::Piece(mesh->Name(),
												 {
													 .fontSize = 16.f,
													 .fontType = FontType::Regular,
												 }),
									 {
										 .size = {Grow, Shrink},
										 .padding = {5.f, 0.f, 0.f, 0.f},
									 }),
						}));
				}
			}

			return new Element(
				{
					.padding = {10.f, 10.f, 10.f, 10.f},
					.spacing = 10.f,
					.backgroundColor = {0.1f, 0.1f, 0.1f, 1.f},
					.direction = Axis::Vertical,
					.scrollable = true,
				},
				elements);
		}

		~MeshList() override = default;
	};

	class MaterialList final : public ReadOnlyTemplate<std::vector<Graphics::Material*>> {
	public:
		Element* Build(const std::vector<Graphics::Material*>& materials) override {
			std::vector<Element*> elements;
			elements.reserve(materials.size());
			for (const auto* material : materials) {
				if (material) {
					elements.emplace_back(new Element(
						{
							.size = {Grow, Shrink},
							.padding = {5.f, 5.f, 5.f, 5.f},
							.spacing = 5.f,
							.cornerRadius = 5.f,
						},
						{
							new Element({
								.size = {100.f, 100.f},
								.backgroundColor = {0.f, 0.f, 0.f, 1.f},
							}),
							new Text(Text::Piece(material->Name(),
												 {
													 .fontSize = 16.f,
													 .fontType = FontType::Regular,
												 }),
									 {
										 .size = {Grow, Shrink},
										 .padding = {5.f, 0.f, 0.f, 0.f},
									 }),
						}));
				}
			}

			return new Element(
				{
					.padding = {10.f, 10.f, 10.f, 10.f},
					.spacing = 10.f,
					.backgroundColor = {0.1f, 0.1f, 0.1f, 1.f},
					.direction = Axis::Vertical,
					.scrollable = true,
				},
				elements);
		}

		~MaterialList() override = default;
	};

	class TextureList final : public ReadOnlyTemplate<std::vector<Graphics::Texture*>> {
	public:
		Element* Build(const std::vector<Graphics::Texture*>& textures) override {
			std::vector<Element*> elements;
			elements.reserve(textures.size());
			for (const auto* texture : textures) {
				if (texture) {
					elements.emplace_back(new Element(
						{
							.size = {Grow, Shrink},
							.padding = {5.f, 5.f, 5.f, 5.f},
							.spacing = 5.f,
							.cornerRadius = 5.f,
						},
						{
							new Image(texture->ImId(),
								  {
									  .size = {100.f, 100.f},
									  .backgroundColor = {0.f, 0.f, 0.f, 1.f},
								  }
							),
							new Text(Text::Piece(
								texture->Name(),
								{
									 .fontSize = 16.f,
									 .fontType = FontType::Regular,
								 }),
								 {
									 .size = {Grow, Shrink},
									 .padding = {5.f, 0.f, 0.f, 0.f},
								 }),
						}));
				}
			}

			return new Element(
				{
					.padding = {10.f, 10.f, 10.f, 10.f},
					.spacing = 10.f,
					.backgroundColor = {0.1f, 0.1f, 0.1f, 1.f},
					.direction = Axis::Vertical,
					.scrollable = true,
				},
				elements);
		}

		~TextureList() override = default;
	};

	class PrefabList final : public ReadOnlyTemplate<std::vector<Asset::Prefab*>> {
	public:
		Element* Build(const std::vector<Asset::Prefab*>& prefabs) override {
			std::vector<Element*> elements;
			elements.reserve(prefabs.size());
			for (const auto* prefab : prefabs) {
				if (prefab) {
					elements.emplace_back(new Button(
						{
							.size = {Grow, Shrink},
							.padding = {5.f, 5.f, 5.f, 5.f},
							.spacing = 5.f,
							.cornerRadius = 5.f,
						},
						[prefab] -> bool {
							prefab->Load();
							return true;
						},
						{
							new Element({
								.size = {100.f, 100.f},
								.backgroundColor = {0.f, 0.f, 0.f, 1.f},
							}),
							new Text(Text::Piece(
								prefab->Name(),
								{
									 .fontSize = 16.f,
									 .fontType = FontType::Regular,
								 }),
								 {
									 .size = {Grow, Shrink},
									 .padding = {5.f, 0.f, 0.f, 0.f},
								 }),
						}));
				}
			}

			return new Element(
				{
					.padding = {10.f, 10.f, 10.f, 10.f},
					.spacing = 10.f,
					.backgroundColor = {0.1f, 0.1f, 0.1f, 1.f},
					.direction = Axis::Vertical,
				},
				elements);
		}

		~PrefabList() override = default;
	};
}