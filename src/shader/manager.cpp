//
// Created by radue on 2/7/2025.
//

#include "manager.h"

#include "IconsFontAwesome6.h"
#include "imgui_impl_vulkan.h"
#include "gui/elements/button.h"
#include "gui/elements/center.h"

#include "gui/elements/column.h"
#include "gui/elements/dockable.h"
#include "gui/elements/row.h"
#include "gui/elements/scrollable.h"
#include "gui/elements/table.h"
#include "gui/elements/text.h"
#include "gui/templates/fileButton.h"
#include "gui/templates/inspector.h"

namespace Shader {
	Manager::Manager(std::filesystem::path defaultSearchPath)
		: m_defaultSearchPath(std::move(defaultSearchPath)) {
		if (GUI::g_manager)
		{
			m_fileButtonTemplate = std::make_unique<GUI::FileButton>(
				[this](const std::filesystem::path &path) {
					if (is_directory(path)) {
						ResetElement("shaderManager");
						m_currentPath = path;
					} else {
						if (const auto shader = Get(path); m_selectedShader != shader) {
							ResetElement("shaderInspector");
							m_selectedShader = shader;
						}
					}
				}
			);
			m_shaderInspectorTemplate = std::make_unique<GUI::ShaderInspector>();
		}
		m_currentPath = m_defaultSearchPath;
	}

	void Manager::OnGUIAttach() {
		m_guiBuilder["shaderManager"] = [this] {
			std::vector<GUI::Element*> images;

			for (const auto &entry : std::filesystem::directory_iterator(m_currentPath)) {
				images.emplace_back(m_fileButtonTemplate->Build(new std::filesystem::path(entry.path())));
			}

			std::vector<GUI::Element*> pathButtons = {
				new GUI::ButtonArea(
					new GUI::Text(ICON_FA_ARROW_LEFT "  back"),
					[this] {
						if (m_currentPath.parent_path() != std::filesystem::path(L"")) {
							ResetElement("shaderManager");
							m_currentPath = m_currentPath.parent_path();
						}
					},
					10.f
				),
			};

			auto path = std::filesystem::path(L"");
			for (const auto &entry : m_currentPath) {
				path /= entry;
				pathButtons.emplace_back(new GUI::ButtonArea(
					new GUI::Text(entry.string()),
					[this, path] {
						if (path == m_currentPath) return;
						ResetElement("shaderManager");
						m_currentPath = path;
					},
					10.f
				));
				pathButtons.emplace_back(new GUI::Center(new GUI::Text("/")));
			}

			return new GUI::Dockable(
				ICON_FA_BRUSH "   Shader Manager",
				new GUI::Column(
					{
						new GUI::Row(
							pathButtons,
							10.f
						),
						new GUI::Table(
							images,
							10.f
						)
					},
					10.f
				),
				10.f,
				new GUI::ContextMenu(
					{
						{
							"Create folder", [this] {
								create_directory(m_currentPath / "test.txt");
							}
						}
					}
				)
			);
		};

		m_guiBuilder["shaderInspector"] = [this] {
			return m_shaderInspectorTemplate->Build(m_selectedShader);
		};
	}

	Core::Shader * Manager::Get(const std::filesystem::path &path) {
		if (m_shaders.contains(path)) {
			return m_shaders[path].get();
		}
		m_shaders[path] = std::make_unique<Core::Shader>(path);
		return m_shaders[path].get();
	}
}
