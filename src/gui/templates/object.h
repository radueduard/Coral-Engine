//
// Created by radue on 2/18/2025.
//

#pragma once

#include "gui/layer.h"
#include "components/object.h"
#include "gui/elements/center.h"
#include "gui/elements/column.h"
#include "gui/elements/dockable.h"
#include "gui/elements/separator.h"
#include "gui/elements/text.h"

namespace GUI {
	class InspectorPanel final : public GUI::Layer {
	public:
		InspectorPanel() : m_selectedObject(nullptr) {}
		~InspectorPanel() override = default;

		void OnGUIAttach() override {
			m_guiObject = std::make_unique<GUI::Dockable>(
				"Inspector",
				new GUI::Column(
					"Vertical Layout",
					std::vector<GUI::Element*>
					{
						new GUI::Text("Inspector",
							GUI::Text::Style {
								.color = glm::vec4 { 0.5f, 0.5f, 0.5f, 1.f },
								.fontSize = 20.f,
								.fontType = GUI::FontType::Black
							}
						),
						new GUI::Separator(),
						new Expanded (
							new GUI::Center(
								new GUI::Text("Select an object to inspect"),
								true,
								true
							)
						),
					},
					10.f
				),
				glm::vec2 { 10.f, 10.f }
			);
		}
		void OnGUIDetach() override {
			m_selectedObject = nullptr;
		}

	private:
		mgv::Object* m_selectedObject;
    };
}
