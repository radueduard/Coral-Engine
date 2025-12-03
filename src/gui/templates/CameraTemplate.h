//
// Created by radue on 3/2/2025.
//


#pragma once
#include "template.h"
#include "ecs/components/camera.h"

#include "gui/reef.h"

namespace Coral::Reef {
	class CameraTemplate final: public ReadWriteTemplate<ECS::Camera> {
	public:
		CameraTemplate() = default;
		~CameraTemplate() override = default;

		Element* Build(ECS::Camera &data) override {
			std::array labels{
				ImGui::ImLabel{.text = "N",
							   .font = Context::GUIManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{161 / 255.f, 163 / 255.f, 44 / 255.f, 1.f}},
				ImGui::ImLabel{.text = "F",
							   .font = Context::GUIManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{163 / 255.f, 58 / 255.f, 44 / 255.f, 1.f}},
			};

			const Text::Style titleStyle {
				.color = { 0.8f, 0.8f, 0.8f, 1.f },
				.fontSize = 20.f,
				.fontStyle = FontType::Black
			};

			return new Element({
					.size = { Grow, Shrink },
					.padding = { 10.f, 10.f, 10.f, 10.f },
					.cornerRadius = 10.f,
					.backgroundColor = { .1f, .1f, .1f, 1.f },
					.direction = Axis::Vertical,
				},
				{
					new Text(" " ICON_FA_CAMERA "    Camera Settings", titleStyle, { .size = { Grow, 20.f } }),
					new Separator(),
					new LabeledRow(
						new Text("Primary"),
						new Checkbox(
							"Primary",
							data.m_primary,
							[&data] (const bool value) {
								if (value) {
									ECS::SceneManager::Get().GetLoadedScene().MainCamera().Primary() = false;
									data.Primary() = true;
								} else {
									data.Primary() = false;
									ECS::SceneManager::Get().GetLoadedScene().Root().Get<ECS::Camera>().Primary() = true;
								}
							},
							Checkbox::DefaultStyle()
								.withSize({ 23.f, Grow })
						),
						Style()
							.withSize({ Grow, 23.f })
					),
					new DropDown(
						"Camera Type",
						&data.GetProjectionData().type,
						{},
						DropDownDefaultStyle()
							.withSize({ Grow, 23.f })
					),
					new Conditional(
						[&data] {
							static ECS::Camera::Type lastType = data.GetProjectionData().type;
							if (lastType != data.GetProjectionData().type) {
								lastType = data.GetProjectionData().type;
								switch (lastType) {
									case ECS::Camera::Type::Orthographic:
									data.m_projectionData.data.orthographic = {};
									break;
								case ECS::Camera::Type::Perspective:
									data.m_projectionData.data.perspective = {};
									break;
								default:
									break;
								}
							}
							return static_cast<u8>(data.GetProjectionData().type);
						},
						{
							new Element({
								.direction = Axis::Vertical,
							}, {
								new LabeledRow {
									new Text("left"),
									new Drag<>(
										"Left",
										&data.m_projectionData.data.orthographic.left,
										0.1f,
										0.f,
										100.f,
										&data.m_changed,
										std::nullopt,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									Style()
										.withSize({ Grow, 23.f })
								},
								new LabeledRow {
									new Text("right"),
									new Drag<>(
										"Right",
										&data.m_projectionData.data.orthographic.right,
										0.1f,
										0.f,
										100.f,
										&data.m_changed,
										std::nullopt,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									Style()
										.withSize({ Grow, 23.f })
								},
								new LabeledRow {
									new Text("top"),
									new Drag<f32, 1>(
										"Top",
										&data.m_projectionData.data.orthographic.top,
										0.1f,
										0.0f,
										100.0f,
										&data.m_changed,
										std::nullopt,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									Style()
										.withSize({ Grow, 23.f })
								},
								new LabeledRow {
									new Text("bottom"),
									new Drag<f32, 1>(
										"Bottom",
										&data.m_projectionData.data.orthographic.bottom,
										0.1f,
										0.0f,
										100.0f,
										&data.m_changed,
										std::nullopt,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									Style()
										.withSize({ Grow, 23.f })
								},
								new LabeledRow {
									new Text("projection planes"),
									new Drag<f32, 2>(
										"Projection planes",
										&data.m_projectionData.data.orthographic.near,
										0.1f,
										0.0f,
										100.0f,
										&data.m_changed,
										labels,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									Style()
										.withSize({ Grow, 23.f })
								},
							}),
							new Element({
								.direction = Axis::Vertical,
							}, {
								new LabeledRow {
									new Text("fov"),
									new Drag<>(
										"Fov",
										&data.m_projectionData.data.perspective.fov,
										1.f,
										40.0f,
										140.0f,
										&data.m_changed,
										std::nullopt,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									Style()
										.withSize({ Grow, 23.f })
								},
								new LabeledRow {
									new Text("projection planes"),
									new Drag<f32, 2>(
										"Projection planes",
										&data.m_projectionData.data.perspective.near,
										0.1f,
										0.0f,
										100.0f,
										&data.m_changed,
										labels,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									Style()
										.withSize({ Grow, 23.f })
								},
							})
						}
					),
				}
			);
		}
	};
}
