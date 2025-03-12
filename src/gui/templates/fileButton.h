//
// Created by radue on 2/22/2025.
//


#pragma once
#include <filesystem>
#include <iostream>

#include "template.h"
#include "graphics/objects/texture.h"
#include "gui/manager.h"
#include "gui/elements/button.h"
#include "gui/elements/column.h"
#include "gui/elements/image.h"
#include "gui/elements/text.h"
#include "shader/shader.h"

namespace mgv {
	class Texture;
}

namespace GUI {
	class FileButton final : public Template<std::filesystem::path> {
	public:
		explicit FileButton(std::function<void(const std::filesystem::path&)> onClick) : m_onClick(std::move(onClick)) {
			m_iconTextures.resize(Core::Shader::Type::Count + 4);
			m_iconTextures[Core::Shader::Type::Vertex] = mgv::Texture::FromFile("assets/icons/icon_vert.png");
			m_iconTextures[Core::Shader::Type::TessellationControl] = mgv::Texture::FromFile("assets/icons/icon_tesc.png");
			m_iconTextures[Core::Shader::Type::TessellationEvaluation] = mgv::Texture::FromFile("assets/icons/icon_tese.png");
			m_iconTextures[Core::Shader::Type::Geometry] = mgv::Texture::FromFile("assets/icons/icon_geom.png");
			m_iconTextures[Core::Shader::Type::Fragment] = mgv::Texture::FromFile("assets/icons/icon_frag.png");
			m_iconTextures[Core::Shader::Type::Task] = mgv::Texture::FromFile("assets/icons/icon_task.png");
			m_iconTextures[Core::Shader::Type::Mesh] = mgv::Texture::FromFile("assets/icons/icon_mesh.png");
			m_iconTextures[Core::Shader::Type::Compute] = mgv::Texture::FromFile("assets/icons/icon_comp.png");
			m_iconTextures[Core::Shader::Type::Count] = mgv::Texture::FromFile("assets/icons/icon_spv.png");
			m_iconTextures[Core::Shader::Type::Count + 1] = mgv::Texture::FromFile("assets/icons/icon_dir.png");
			m_iconTextures[Core::Shader::Type::Count + 2] = mgv::Texture::FromFile("assets/icons/icon_text.png");
			m_iconTextures[Core::Shader::Type::Count + 3] = mgv::Texture::FromFile("assets/icons/icon_bin.png");

			m_textureIds.reserve(Core::Shader::Type::Count + 4);
			for (const auto& texture : m_iconTextures) {
				m_textureIds.emplace_back(ImGui_ImplVulkan_AddTexture(
					*texture->Sampler(),
					*texture->ImageView(1, 1),
					static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)
				));
			}
		}

		~FileButton() override {
			for (const auto textureId : m_textureIds) {
				ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(textureId));
			}
		}

		Element *Build(std::filesystem::path *data) override {
			const auto isDirectory = std::filesystem::is_directory(*data);
			std::string name = data->filename().string();

			ImTextureID texture = nullptr;
			if (isDirectory) {
				name += "/";
				texture = m_textureIds[Core::Shader::Type::Count + 1];
			} else {
				const auto extension = data->extension().string();
				if (extension == ".vert") {
					texture = m_textureIds[Core::Shader::Type::Vertex];
				} else if (extension == ".tesc") {
					texture = m_textureIds[Core::Shader::Type::TessellationControl];
				} else if (extension == ".tese") {
					texture = m_textureIds[Core::Shader::Type::TessellationEvaluation];
				} else if (extension == ".geom") {
					texture = m_textureIds[Core::Shader::Type::Geometry];
				} else if (extension == ".frag") {
					texture = m_textureIds[Core::Shader::Type::Fragment];
				} else if (extension == ".task") {
					texture = m_textureIds[Core::Shader::Type::Task];
				} else if (extension == ".mesh") {
					texture = m_textureIds[Core::Shader::Type::Mesh];
				} else if (extension == ".comp") {
					texture = m_textureIds[Core::Shader::Type::Compute];
				} else if (extension == ".spv") {
					texture = m_textureIds[Core::Shader::Type::Count];
				} else {
					if (is_character_file(*data)) {
						texture = m_textureIds[Core::Shader::Type::Count + 2];
					} else {
						texture = m_textureIds[Core::Shader::Type::Count + 3];
					}
				}
			}

			return new GUI::ButtonArea(
				new GUI::Column(
					{
						new GUI::Image(
							texture,
							{ 80, 80 }
						),
						new GUI::Text(
							name,
							{
								.color = Math::Color { 1.f, 1.f, 1.f, 1.f },
								.fontSize = 13.f,
								.fontType = FontType::Medium
							}
						)
					},
					10.f
				),
				[data, this] {
					m_onClick(*data);
				},
				10.f
			);
		}
	private:
		std::vector<std::unique_ptr<mgv::Texture>> m_iconTextures;
		std::vector<ImTextureID> m_textureIds;

		std::function<void(const std::filesystem::path&)> m_onClick;
	};
}
