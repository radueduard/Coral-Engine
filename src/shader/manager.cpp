//
// Created by radue on 2/7/2025.
//

#include "manager.h"


namespace Coral::Shader {
	Manager::Manager(std::filesystem::path defaultSearchPath)
		: m_defaultSearchPath(std::move(defaultSearchPath)) {
		// if (GUI::g_manager)
		// {
		// 	m_fileButtonTemplate = std::make_unique<GUI::FileButton>(
		// 		[this](const std::filesystem::path &path) {
		// 			if (is_directory(path)) {
		// 				ResetElement("shaderManager");
		// 				m_currentPath = path;
		// 			} else {
		// 				if (const auto shader = Get(path); m_selectedShader != shader) {
		// 					ResetElement("shaderInspector");
		// 					m_selectedShader = shader;
		// 				}
		// 			}
		// 		}
		// 	);
		// 	m_shaderInspectorTemplate = std::make_unique<GUI::ShaderInspector>();
		// }
		m_currentPath = m_defaultSearchPath;
	}

	void Manager::OnGUIAttach() {
		// m_guiBuilder["shaderManager"] = [this] {
		// 	std::vector<GUI::Element*> images;
		//
		// 	for (auto &entry : std::filesystem::directory_iterator(m_currentPath)) {
		// 		images.emplace_back(m_fileButtonTemplate->Build(entry.path()));
		// 	}
		//
		// 	std::vector<GUI::Element*> pathButtons = {
		// 		new GUI::ButtonArea(
		// 			new GUI::Text(ICON_FA_ARROW_LEFT "  back"),
		// 			[this] {
		// 				if (m_currentPath.parent_path() != std::filesystem::path(L"")) {
		// 					ResetElement("shaderManager");
		// 					m_currentPath = m_currentPath.parent_path();
		// 				}
		// 			},
		// 			{ 10.f, 10.f }
		// 		),
		// 	};
		//
		// 	auto path = std::filesystem::path(L"");
		// 	for (const auto &entry : m_currentPath) {
		// 		path /= entry;
		// 		pathButtons.emplace_back(new GUI::ButtonArea(
		// 			new GUI::Text(entry.string()),
		// 			[this, path] {
		// 				if (path == m_currentPath) return;
		// 				ResetElement("shaderManager");
		// 				m_currentPath = path;
		// 			},
		// 			{ 10.f, 10.f }
		// 		));
		// 		pathButtons.emplace_back(new GUI::Center(new GUI::Text("/")));
		// 	}
		//
		// 	return new GUI::Dockable(
		// 		ICON_FA_BRUSH "   Shader Manager",
		// 		new GUI::Column(
		// 			{
		// 				new GUI::Row(
		// 					pathButtons,
		// 					10.f
		// 				),
		// 				new GUI::Table(
		// 					images,
		// 					10.f
		// 				)
		// 			},
		// 			10.f
		// 		),
		// 		{ 10.f, 10.f },
		// 		new GUI::ContextMenu(
		// 			{
		// 				{
		// 					"Create folder", [this] {
		// 						create_directory(m_currentPath / "test.txt");
		// 					}
		// 				}
		// 			}
		// 		)
		// 	);
		// };
		//
		// m_guiBuilder["shaderInspector"] = [this] () -> GUI::Element* {
		// 	if (m_selectedShader == nullptr) {
		// 		return new GUI::Dockable(
		// 			ICON_FA_INFO "   Shader Inspector",
		// 			new GUI::Text("No shader selected", GUI::Text::Style{ { 0.8f, 0.8f, 0.8f, 1.f }, 20.f, GUI::FontType::Black }),
		// 			{ 10.f, 10.f }
		// 		);
		// 	}
		// 	return m_shaderInspectorTemplate->Build(*m_selectedShader);
		// };
	}

	Core::Shader * Manager::Get(const std::filesystem::path &path) {
		if (!m_shaders.contains(path)) {
			m_shaders[path] = std::make_unique<Core::Shader>(path);
		}
		return m_shaders[path].get();
	}
}
