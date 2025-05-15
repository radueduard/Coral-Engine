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
							   .font = GlobalManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{161 / 255.f, 163 / 255.f, 44 / 255.f, 1.f}},
				ImGui::ImLabel{.text = "F",
							   .font = GlobalManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{163 / 255.f, 58 / 255.f, 44 / 255.f, 1.f}},
			};

			constexpr Text::Style labelStyle {
				.color = { 0.8f, 0.8f, 0.8f, 1.f },
				.fontSize = 15.f,
				.fontType = FontType::Bold,
				.minSize = { 60.f, 0.f }
			};

			return new Element({
					.size = { Grow, Shrink },
					.padding = { 10.f, 10.f, 10.f, 10.f },
					.cornerRadius = 10.f,
					.backgroundColor = { .1f, .1f, .1f, 1.f },
					.direction = Vertical,
				},
				{
					Text::Builder({ .size = { 0.f, 20.f } })
						.Add(
							" " ICON_FA_CAMERA "    Camera Settings",
							Text::Style{
								{ 0.8f, 0.8f, 0.8f, 1.f },
								20.f,
								FontType::Black
							})
						.Build(),
					new Separator(),
					new LabeledRow(
						new Text(Text::Piece {"Primary", labelStyle}, { .size = { Shrink, Grow } }),
						new Checkbox(
							data.m_primary,
							[&data] (const bool value) {
								if (value) {
									ECS::Scene::Get().MainCamera().Primary() = false;
									data.Primary() = true;
								} else {
									data.Primary() = false;
									ECS::Scene::Get().Root().Get<ECS::Camera>().Primary() = true;
								}
							}, { .size = { 23, Grow } }),
						{ .size = { Grow, 23.f } }
					),
					new DropDown(
						"Camera Type",
						&data.GetProjectionData().type,
						{},
						Style {
							.size = { Grow, 23.f },
							// .padding = { 10.f, 0.f, 0.f, 0.f }
						}
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
							return data.m_projectionData.type == ECS::Camera::Type::Orthographic;
						},
						new Element({
							.direction = Vertical,
						}, {
							new LabeledRow {
								new Text(Text::Piece {"left", labelStyle}, { .size = { Shrink, Grow } }),
								new Drag<f32, 1>("Left", &data.m_projectionData.data.orthographic.left, 0.1f, 0.f, 100.f, std::nullopt, { .size = { 250.f, Grow } }),
								{ .size = { 0.f, 23.f } }
							},
							new LabeledRow {
								new Text(Text::Piece {"right", labelStyle}, { .size = { Shrink, Grow } }),
								new Drag<f32, 1>("Right", &data.m_projectionData.data.orthographic.right, 0.1f, 0.f, 100.f, std::nullopt, { .size = { 250.f, Grow } }),
								{ .size = { 0.f, 23.f } }
							},
							new LabeledRow {
								new Text(Text::Piece {"top", labelStyle}, { .size = { Shrink, Grow } }),
								new Drag<f32, 1>("Top", &data.m_projectionData.data.orthographic.top, 0.1f, 0.0f, 100.0f, std::nullopt, { .size = { 250.f, Grow } }),
								{ .size = { 0.f, 23.f } }
							},
							new LabeledRow {
								new Text(Text::Piece {"bottom", labelStyle}, { .size = { Shrink, Grow } }),
								new Drag<f32, 1>("Bottom", &data.m_projectionData.data.orthographic.bottom, 0.1f, 0.0f, 100.0f, std::nullopt, { .size = { 250.f, Grow } }),
								{ .size = { 0.f, 23.f } }
							},
							new LabeledRow {
								new Text(Text::Piece {"projection planes", labelStyle}, { .size = { Shrink, Grow } }),
								new Drag<f32, 2>("Projection planes", &data.m_projectionData.data.orthographic.near, 0.1f, 0.0f, 100.0f, labels, { .size = { 250.f, Grow } }),
								{ .size = { 0.f, 23.f } }
							},
						}),
						new Element({
							.direction = Vertical,
						}, {
							new LabeledRow {
								new Text(Text::Piece {"fov", labelStyle}, { .size = { Shrink, Grow } }),
								new Drag<f32, 1>("Fov", &data.m_projectionData.data.perspective.fov, 1.f, 40.0f, 140.0f, std::nullopt, { .size = { 250.f, Grow } }),
								{ .size = { 0.f, 23.f } }
							},
							new LabeledRow {
								new Text(Text::Piece {"projection planes", labelStyle}, { .size = { Shrink, Grow } }),
								new Drag<f32, 2>("Projection planes", &data.m_projectionData.data.perspective.near, 0.1f, 0.0f, 100.0f, labels, { .size = { 250.f, Grow } }),
								{ .size = { 0.f, 23.f } }
							},
						})
					),
				}
			);
		}
	};
}
