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
							   .font = Context::GUIManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{161 / 255.f, 161 / 255.f, 161 / 255.f, 1.f}},
				ImGui::ImLabel{.text = "1",
							   .font = Context::GUIManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{161 / 255.f, 161 / 255.f, 161 / 255.f, 1.f}},
				ImGui::ImLabel{.text = "2",
							   .font = Context::GUIManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{161 / 255.f, 161 / 255.f, 161 / 255.f, 1.f}},
			};

			std::array colorLabels{
				ImGui::ImLabel{.text = "R",
							   .font = Context::GUIManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{ 0.8f, 0.f, 0.f, 1.f }},
				ImGui::ImLabel{.text = "G",
							   .font = Context::GUIManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{ 0.f, 0.8f, 0.f, 1.f }},
				ImGui::ImLabel{.text = "B",
							   .font = Context::GUIManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{ 0.f, 0.f, 0.8f, 1.f }},
			};

			std::array angleLabels{
				ImGui::ImLabel{.text = "I",
							   .font = Context::GUIManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{161 / 255.f, 163 / 255.f, 44 / 255.f, 1.f}},
				ImGui::ImLabel{.text = "O",
							   .font = Context::GUIManager().GetFont(FontType::Black, 16.f),
							   .color = ImVec4(1.f, 1.f, 1.f, 1.f),
							   .embedded = true,
							   .backgroundColor = ImVec4{163 / 255.f, 58 / 255.f, 44 / 255.f, 1.f}},
			};

			const Text::Style labelStyle = Text::Style()
				.withColor( { 0.8f, 0.8f, 0.8f, 1.f } )
				.withFontSize(15.f)
				.withFontStyle(FontType::Bold);

			return new Element({
					.size = { Grow, Shrink },
					.padding = { 10.f, 10.f, 10.f, 10.f },
					.cornerRadius = 10.f,
					.backgroundColor = { .1f, .1f, .1f, 1.f },
					.direction = Axis::Vertical,
				},
				{
					new Text(
						" " ICON_FA_LIGHTBULB "    Light Settings",
						Text::Style()
							.withColor(Colors::grey[300])
							.withFontSize(20.f)
							.withFontStyle(FontType::Black)
							.withVerticalAlignment(Text::VerticalAlignment::Middle)
							.withHorizontalAlignment(Text::HorizontalAlignment::Left),
						{ .size = { Grow, 23.f } }
					),
					new Separator(),
					new DropDown(
						"Light Type",
						&data.type,
						{},
						DropDownDefaultStyle()
							.withSize({ Grow, 23.f })
					),
					new Conditional(
						[&data] {
							static ECS::LightType lastType = data.type;
							if (lastType != data.type) {
								lastType = data.type;
							// 	switch (lastType) {
							// 	case ECS::LightType::Point:
							// 		break;
							// 	case ECS::LightType::Directional:
							// 		break;
							// 	case ECS::LightType::Spot:
							// 		break;
							// 	default:
							// 		break;
							// 	}
							}
							return static_cast<u8>(lastType);
						},
						&data.m_changed,
						{
							new Element({
								.direction = Axis::Vertical,
							}, {
								new LabeledRow {
									new Text("color"),
									new Drag<f32, 3>(
										"Color",
										{ &data.color.r, &data.color.g, &data.color.b },
										0.01f,
										{ 0.f, 0.f, 0.f },
										{ 1.f, 1.f, 1.f },
										&data.m_changed,
										colorLabels,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									{ .size = { 0.f, 23.f } }
								},
								new LabeledRow {
									new Text("attenuation"),
									new Drag<f32, 3>(
										"Attenuation",
										{ &data.attenuation.x, &data.attenuation.y, &data.attenuation.z },
										0.01f,
										{ 0.f, 0.f, 0.f },
										{ 1.f, 1.f, 1.f },
										&data.m_changed,
										attenuationLabels,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									{ .size = { 0.f, 23.f } }
								},
								new LabeledRow {
									new Text("range"),
									new Drag<>(
										"Range",
										{ &data.range },
										0.1f,
										{ 0.f },
										{ 100.f },
										&data.m_changed,
										std::nullopt,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									{ .size = { 0.f, 23.f } }
								},
								new LabeledRow {
									new Text("intensity"),
									new Drag<>(
										"Intensity",
										{ &data.intensity },
										0.1f,
										{ 0.f },
										{ 100.f },
										&data.m_changed,
										std::nullopt,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									{ .size = { 0.f, 23.f } }
								},
							}),
							new Element({
								.direction = Axis::Vertical,
							}, {
								new LabeledRow {
									new Text("color"),
									new Drag<f32, 3>(
										"Color",
										{ &data.color.r, &data.color.g, &data.color.b },
										0.01f,
										{ 0.f, 0.f, 0.f },
										{ 1.f, 1.f, 1.f },
										&data.m_changed,
										colorLabels,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									{ .size = { 0.f, 23.f } }
								},
								new LabeledRow {
									new Text("intensity"),
									new Drag<>(
										"Intensity",
										{ &data.intensity },
										0.1f,
										{ 0.f },
										{ 100.f },
										&data.m_changed,
										std::nullopt,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									{ .size = { 0.f, 23.f } }
								},
							}),
							new Element({
								.direction = Axis::Vertical,
							}, {
								new LabeledRow {
									new Text("color"),
									new Drag<f32, 3>(
										"Color",
										{ &data.color.r, &data.color.g, &data.color.b },
										0.01f,
										{ 0.f, 0.f, 0.f },
										{ 1.f, 1.f, 1.f },
										&data.m_changed,
										colorLabels,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })),
									{ .size = { 0.f, 23.f } }
								},
								new LabeledRow {
									new Text("intensity"),
									new Drag<f32, 1>(
										"Intensity",
										{ &data.intensity },
										0.1f,
										{ 0.f },
										{ 100.f },
										&data.m_changed,
										std::nullopt,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									{ .size = { 0.f, 23.f } }
								},
								new LabeledRow {
									new Text("range"),
									new Drag<f32, 1>(
										"Range",
										{ &data.range, },
										0.1f,
										{ 0.f },
										{ 100.f },
										&data.m_changed,
										std::nullopt,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
									{ .size = { 0.f, 23.f } }
								},
								new LabeledRow {
									new Text("angles"),
									new Drag<f32, 2>(
										"Angles",
										{ &data.innerAngle, &data.outerAngle },
										0.01f,
										{ 0.f, 0.f },
										{ 90.f, 180.f },
										&data.m_changed,
										angleLabels,
										DragDefaultStyle()
											.withSize({ 250.f, Grow })
									),
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