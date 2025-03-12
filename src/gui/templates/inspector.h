//
// Created by radue on 2/18/2025.
//

#pragma once

#include "components/object.h"
#include "IconsFontAwesome6.h"
#include "template.h"
#include "gui/elements/center.h"
#include "gui/elements/column.h"
#include "gui/elements/dockable.h"
#include "gui/elements/text.h"

#include <shader/shader.h>

#include "cameraSettings.h"
#include "gui/elements/drag.h"
#include "gui/elements/inputField.h"
#include "gui/elements/separator.h"

namespace GUI {
	class ObjectInspector final : public GUI::Template<mgv::Object> {
	public:
		ObjectInspector() {
			m_cameraSettings = std::make_unique<CameraSettings>();
		}

		GUI::Element *Build(mgv::Object *data) override {
			GUI::Element* innerElement = nullptr;
			if (data != nullptr) {
				std::array labels {
					ImGui::ImLabel {
						.text = "X",
						.font = GUI::Manager::GetFont(FontType::Black, 16.f),
						.color = ImVec4(1.f, 1.f, 1.f, 1.f),
						.embedded = true,
						.backgroundColor = ImVec4 { 1.f, 0.f, 0.f, 1.f }
					},
					ImGui::ImLabel {
						.text = "Y",
						.font = GUI::Manager::GetFont(FontType::Black, 16.f),
						.color = ImVec4(1.f, 1.f, 1.f, 1.f),
						.embedded = true,
						.backgroundColor = ImVec4 { 0, .65f, .13f, 1.f }
					},
					ImGui::ImLabel {
						.text = "Z",
						.font = GUI::Manager::GetFont(FontType::Black, 16.f),
						.color = ImVec4(1.f, 1.f, 1.f, 1.f),
						.embedded = true,
						.backgroundColor = ImVec4 { 0.f, .13f, .65f, 1.f }
					}
				};

				std::vector<Element*> children {
					new LabeledRow("Name", new GUI::InputField("Name", &(data->m_name))),
					new GUI::Text(
						ICON_FA_LOCATION_CROSSHAIRS "   Transform",
						GUI::Text::Style{
							{ 0.8f, 0.8f, 0.8f, 1.f },
							20.f,
							FontType::Black
						}
					),
					new LabeledRow("Position", new Drag<float, 3>("Position", reinterpret_cast<float*>(&data->position), 10.f, -1000.f, 1000.f, labels)),
					new LabeledRow("Rotation", new Drag<float, 3>("Rotation", reinterpret_cast<float*>(&data->rotation), 1.f, -180.f, 180.f, labels)),
					new LabeledRow("Scale", new Drag<float, 3>("Scale", reinterpret_cast<float*>(&data->scale), 0.1f, -10.f, 10.f, labels)),
					new GUI::Separator(),
				};

				for (auto* component : data->Components()) {
					if (typeid(*component) == typeid(mgv::Camera)) {
						children.emplace_back(m_cameraSettings->Build(dynamic_cast<mgv::Camera*>(component)));
					}
				}

				innerElement = new GUI::Column(
					children,
					10.f
				);
			} else {
				innerElement = new GUI::Center(
					new GUI::Text("Select an object to inspect"),
					true,
					true
				);
			}

			return new GUI::Dockable(
				ICON_FA_INFO "   Object Inspector",
				innerElement,
				10.f
			);
		}
	private:
		std::unique_ptr<GUI::CameraSettings> m_cameraSettings;
    };

	class ShaderInspector final : public GUI::Template<Core::Shader> {
	public:
		GUI::Element *Build(Core::Shader *data) override {
			GUI::Element* innerElement = nullptr;
			if (data != nullptr) {
				innerElement = new GUI::Column(
					{
						new GUI::Text(
							ICON_FA_LOCATION_CROSSHAIRS "   Shader",
							GUI::Text::Style{
								{ 0.8f, 0.8f, 0.8f, 1.f },
								20.f,
								FontType::Black
							}
						),
						new GUI::Text("Name: " + data->Name()),
						new GUI::Text("Path: " + data->Path().string()),
					},
					10.f
				);
			} else {
				innerElement = new GUI::Center(
					new GUI::Text("Select a shader to inspect"),
					true,
					true
				);
			}

			return new GUI::Dockable(
				ICON_FA_INFO "   Shader Inspector",
				innerElement,
				10.f
			);
		}
	};
}
