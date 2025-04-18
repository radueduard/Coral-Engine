//
// Created by radue on 2/22/2025.
//


#pragma once
#include <filesystem>
#include <iostream>

#include <imgui_impl_vulkan.h>

#include "template.h"
#include "graphics/objects/texture.h"
#include "gui/manager.h"
#include "gui/elements/button.h"
#include "gui/elements/column.h"
#include "gui/elements/image.h"
#include "gui/elements/text.h"
#include "utils/file.h"

namespace Coral {
	class Texture;
}

namespace GUI {
	class FileButton final : public Template<std::filesystem::path> {
	public:
		explicit FileButton(std::function<void(const std::filesystem::path&)> onClick) : m_onClick(std::move(onClick)) {
			for (const auto& [type, path] : Utils::IconPaths)
			{
				int width, height, channels;
				std::vector<glm::u8vec4> pixels;
				Utils::ReadImageData(path, pixels, width, height, channels);
				m_iconTextures[type] = Coral::Texture::Builder()
                    .Name(type)
                    .Data(reinterpret_cast<uint8_t*>(pixels.data()))
                    .Width(width)
					.Height(height)
                    .Format(vk::Format::eR8G8B8A8Unorm)
                    .Build();
			}

			for (const auto& [type, texture] : m_iconTextures) {
				const auto id = ImGui_ImplVulkan_AddTexture(
					*texture->Sampler(),
					*texture->ImageView(),
					static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)
				);
				m_textureIds[type] = id;
			}
		}

		~FileButton() override {
			for (const auto textureId : m_textureIds | std::views::values) {
				ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(textureId));
			}
		}

		Element *Build(std::filesystem::path *data) override {
			const auto isDirectory = std::filesystem::is_directory(*data);
			std::string name = data->filename().string();

			ImTextureID texture = nullptr;
			if (isDirectory) {
				name += "/";
				texture = m_textureIds["Directory"];
			} else {
				const auto extension = data->extension().generic_string().substr(1);
				const auto type = Utils::FileTypes[extension];
				texture = m_textureIds[type];
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
		std::unordered_map<std::string, std::unique_ptr<Coral::Texture>> m_iconTextures;
		std::unordered_map<std::string, ImTextureID> m_textureIds;

		std::function<void(const std::filesystem::path&)> m_onClick;
	};
}
