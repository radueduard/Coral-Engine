//
// Created by radue on 6/29/2025.
//

#pragma once
#include "IconsFontAwesome6.h"
#include "ecs/components/light.h"
#include "ecs/sceneManager.h"
#include "gui/elements/checkbox.h"
#include "gui/elements/conditional.h"
#include "gui/elements/drag.h"
#include "gui/elements/dropDown.h"
#include "gui/elements/labeledRow.h"
#include "gui/elements/separator.h"
#include "gui/elements/text.h"
#include "gui/manager.h"
#include "template.h"

namespace Coral::Reef {
	class LightTemplate final: public ReadWriteTemplate<ECS::Light> {
	public:
		LightTemplate() = default;
		~LightTemplate() override = default;

		Element* Build(ECS::Light &data) override {
			std::array attenuationLabels{
				ImGui::ImLabel{.text = "0",
							   .font = GlobalManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{161 / 255.f, 161 / 255.f, 161 / 255.f, 1.f}},
				ImGui::ImLabel{.text = "1",
							   .font = GlobalManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{161 / 255.f, 161 / 255.f, 161 / 255.f, 1.f}},
				ImGui::ImLabel{.text = "2",
							   .font = GlobalManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{161 / 255.f, 161 / 255.f, 161 / 255.f, 1.f}},
			};

			std::array colorLabels{
				ImGui::ImLabel{.text = "R",
							   .font = GlobalManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{ 0.8f, 0.f, 0.f, 1.f }},
				ImGui::ImLabel{.text = "G",
							   .font = GlobalManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{ 0.f, 0.8f, 0.f, 1.f }},
				ImGui::ImLabel{.text = "B",
							   .font = GlobalManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{ 0.f, 0.f, 0.8f, 1.f }},
			};

			std::array angleLabels{
				ImGui::ImLabel{.text = "I",
							   .font = GlobalManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{161 / 255.f, 163 / 255.f, 44 / 255.f, 1.f}},
				ImGui::ImLabel{.text = "O",
							   .font = GlobalManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{163 / 255.f, 58 / 255.f, 44 / 255.f, 1.f}},
			};

			const Text::Style labelStyle {
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
					.direction = Axis::Vertical,
				},
				{
					Text::Builder({ .size = { 0.f, 20.f } })
						.Add(
							" " ICON_FA_LIGHTBULB "    Light Settings",
							Text::Style{
								{ 0.8f, 0.8f, 0.8f, 1.f },
								20.f,
								FontType::Black
							})
						.Build(),
					new Separator(),
					new DropDown(
						"Light Type",
						&data.m_type,
						{},
						Style {
							.size = { Grow, 23.f },
						}
					),
					new Conditional(
						[&data] {
							static ECS::Light::Type lastType = data.m_type;
							if (lastType != data.m_type) {
								lastType = data.m_type;
								switch (lastType) {
								case ECS::Light::Type::Point:
									data.m_data = ECS::Light::Data(ECS::Light::Point {});
									break;
								case ECS::Light::Type::Directional:
									data.m_data = ECS::Light::Data(ECS::Light::Directional {});
									break;
								case ECS::Light::Type::Spot:
									data.m_data = ECS::Light::Data(ECS::Light::Spot {});
								default:
									break;
								}
							}
							return static_cast<u8>(lastType);
						},
						{
							new Element({
								.direction = Axis::Vertical,
							}, {
								new LabeledRow {
									new Text(Text::Piece {"color", labelStyle}, { .size = { Shrink, Grow } }),
									new Drag<f32, 3>("Color", &data.m_data.point.color.r, 0.01f, 0.f, 1.f, &data.m_changed, colorLabels, { .size = { 250.f, Grow } }),
									{ .size = { 0.f, 23.f } }
								},
								new LabeledRow {
									new Text(Text::Piece {"attenuation", labelStyle}, { .size = { Shrink, Grow } }),
									new Drag<f32, 3>("Attenuation", &data.m_data.point.attenuation.x, 0.01f, 0.f, 1.f, &data.m_changed, attenuationLabels, { .size = { 250.f, Grow } }),
									{ .size = { 0.f, 23.f } }
								},
								new LabeledRow {
									new Text(Text::Piece {"range", labelStyle}, { .size = { Shrink, Grow } }),
									new Drag<f32, 1>("Range", &data.m_data.point.range, 0.1f, 0.f, 100.f, &data.m_changed, std::nullopt, { .size = { 250.f, Grow } }),
									{ .size = { 0.f, 23.f } }
								},
							}),
							new Element({
								.direction = Axis::Vertical,
							}, {
								new LabeledRow {
									new Text(Text::Piece {"color", labelStyle}, { .size = { Shrink, Grow } }),
									new Drag<f32, 3>("Color", &data.m_data.directional.color.r, 0.01f, 0.f, 1.f, &data.m_changed, colorLabels, { .size = { 250.f, Grow } }),
									{ .size = { 0.f, 23.f } }
								},
								new LabeledRow {
									new Text(Text::Piece {"intensity", labelStyle}, { .size = { Shrink, Grow } }),
									new Drag<f32, 1>("Intensity", &data.m_data.directional.intensity, 0.1f, 0.f, 100.f, &data.m_changed, std::nullopt, { .size = { 250.f, Grow } }),
									{ .size = { 0.f, 23.f } }
								},
							}),
							new Element({
								.direction = Axis::Vertical,
							}, {
								new LabeledRow {
									new Text(Text::Piece {"color", labelStyle}, { .size = { Shrink, Grow } }),
									new Drag<f32, 3>("Color", &data.m_data.spot.color.r, 0.01f, 0.f, 1.f, &data.m_changed, colorLabels, { .size = { 250.f, Grow } }),
									{ .size = { 0.f, 23.f } }
								},
								new LabeledRow {
									new Text(Text::Piece {"intensity", labelStyle}, { .size = { Shrink, Grow } }),
									new Drag<f32, 1>("Intensity", &data.m_data.spot.intensity, 0.1f, 0.f, 100.f, &data.m_changed, std::nullopt, { .size = { 250.f, Grow } }),
									{ .size = { 0.f, 23.f } }
								},
								new LabeledRow {
									new Text(Text::Piece {"range", labelStyle}, { .size = { Shrink, Grow } }),
									new Drag<f32, 1>("Range", &data.m_data.spot.range, 0.1f, 0.f, 100.f, &data.m_changed, std::nullopt, { .size = { 250.f, Grow } }),
									{ .size = { 0.f, 23.f } }
								},
								new LabeledRow {
									new Text(Text::Piece {"angles", labelStyle}, { .size = { Shrink, Grow } }),
									new Drag<f32, 2>("Angles", &data.m_data.spot.innerAngle, 0.01f, 0.f, 180.f, &data.m_changed, angleLabels, { .size = { 250.f, Grow } }),
									{ .size = { 0.f, 23.f } }
								},
							})
						}
					),
				}
			);
		}
	};
}