//
// Created by radue on 3/2/2025.
//


#pragma once
#include "template.h"
#include "components/camera.h"
#include "gui/elements/column.h"
#include "gui/elements/conditional.h"
#include "gui/elements/drag.h"

#include "gui/elements/dropDown.h"
#include "gui/elements/labeledRow.h"

namespace GUI {
	class CameraSettings final: public Template<mgv::Camera> {
	public:
		CameraSettings() = default;
		~CameraSettings() override = default;

		Element* Build(mgv::Camera *data) override {
			std::array labels {
				ImGui::ImLabel {
					.text = "N",
					.font = GUI::Manager::GetFont(FontType::Black, 16.f),
					.color = ImVec4(1.f, 1.f, 1.f, 1.f),
					.embedded = true,
					.backgroundColor = ImVec4 {161 / 255.f, 163 / 255.f, 44 / 255.f, 1.f}
				},
				ImGui::ImLabel {
					.text = "F",
					.font = GUI::Manager::GetFont(FontType::Black, 16.f),
					.color = ImVec4(1.f, 1.f, 1.f, 1.f),
					.embedded = true,
					.backgroundColor = ImVec4 { 163 / 255.f, 58 / 255.f, 44 / 255.f, 1.f }
				},
			};

			return new GUI::Column(
				std::vector<Element*> {
					new GUI::Text(ICON_FA_CAMERA "    Camera Settings", GUI::Text::Style{ { 0.8f, 0.8f, 0.8f, 1.f }, 20.f, FontType::Black }),
					new GUI::LabeledRow("Projection Type", new GUI::DropDown("Projection Type", &data->ProjectionData().type)),
					new GUI::Conditional(
						[data] {
							static mgv::Camera::Type lastType = data->ProjectionData().type;
							if (lastType != data->ProjectionData().type) {
								lastType = data->ProjectionData().type;
								switch (lastType) {
									case mgv::Camera::Type::Orthographic:
										data->ProjectionData().data.orthographic = {};
										break;
									case mgv::Camera::Type::Perspective:
										data->ProjectionData().data.perspective = {};
										break;
									default:
										break;
								}
							}
							return data->ProjectionData().type == mgv::Camera::Type::Orthographic;
						},
						new GUI::Column({
						    new GUI::LabeledRow("Left", new Drag<float, 1>("Left", &data->ProjectionData().data.orthographic.left, 0.1f, 0.0f, 100.0f)),
							new GUI::LabeledRow("Right", new GUI::Drag<float, 1>("Right", &data->ProjectionData().data.orthographic.right, 0.1f, 0.0f, 100.0f)),
							new GUI::LabeledRow("Top", new GUI::Drag<float, 1>("Top", &data->ProjectionData().data.orthographic.top, 0.1f, 0.0f, 100.0f)),
							new GUI::LabeledRow("Bottom", new GUI::Drag<float, 1>("Bottom", &data->ProjectionData().data.orthographic.bottom, 0.1f, 0.0f, 100.0f)),
							new GUI::LabeledRow("Projection planes", new GUI::Drag<float, 2>("Near", &data->ProjectionData().data.orthographic.near, 0.1f, 0.0f, 100.0f, labels)),
						}, 10.f),
						new GUI::Column({
							new GUI::LabeledRow("Fov", new GUI::Drag<float, 1>("Fov", &data->ProjectionData().data.perspective.fov, 1.f, 0.0f, 180.0f)),
							new GUI::LabeledRow("Projection planes", new GUI::Drag<float, 2>("Near", &data->ProjectionData().data.perspective.near, 0.1f, 0.0f, 100.0f, labels)),
						}, 10.f)
					),
				},
				10.f
			);
		}

	};
}
