//
// Created by radue on 5/7/2025.
//

#pragma once
#include "IconsFontAwesome6.h"
#include "template.h"
#include "ecs/components/transform.h"
#include "gui/elements/drag.h"
#include "gui/elements/labeledRow.h"
#include "gui/elements/separator.h"

namespace Coral::Reef {
	class TransformTemplate final : public ReadWriteTemplate<ECS::Transform> {
	public:
		TransformTemplate() = default;
		~TransformTemplate() override = default;

		Element* Build(ECS::Transform& data) override {
			std::array labels {
				ImGui::ImLabel {
					.text = "X",
					.font = Context::GUIManager().GetFont(FontType::Black, 16.f),
					.color = ImVec4(1.f, 1.f, 1.f, 1.f),
					.embedded = true,
					.backgroundColor = ImVec4 { 1.f, 0.f, 0.f, 1.f }
				},
				ImGui::ImLabel {
					.text = "Y",
					.font = Context::GUIManager().GetFont(FontType::Black, 16.f),
					.color = ImVec4(1.f, 1.f, 1.f, 1.f),
					.embedded = true,
					.backgroundColor = ImVec4 { 0, .65f, .13f, 1.f }
				},
				ImGui::ImLabel {
					.text = "Z",
					.font = Context::GUIManager().GetFont(FontType::Black, 16.f),
					.color = ImVec4(1.f, 1.f, 1.f, 1.f),
					.embedded = true,
					.backgroundColor = ImVec4 { 0.f, .13f, .65f, 1.f }
				}
			};

			const Text::Style labelStyle {
				.color = { 0.8f, 0.8f, 0.8f, 1.f },
				.fontSize = 15.f,
				.fontStyle = FontType::Bold,
				// .minSize = { 60.f, 0.f }
			};

			return new Element({
					.size = { Grow, Shrink },
					.padding = { 10.f, 10.f, 10.f, 10.f },
					.cornerRadius = 10.f,
					.backgroundColor = { .1f, .1f, .1f, 1.f },
					.direction = Axis::Vertical,
				},
				{
					new Text(
						" " ICON_FA_LOCATION_CROSSHAIRS "   Transform",
						Text::Style{
							{ 0.8f, 0.8f, 0.8f, 1.f },
							20.f,
							FontType::Black
						},
						{ .size = { 0.f, 20.f } }
					),
					new Separator(),
					new LabeledRow {
						new Text("position", labelStyle, { .size = { Grow, Grow } }),
						new Drag<f32, 3>("Position", reinterpret_cast<f32*>(&data.position), 0.5f, -100.f, 100.f, &data.m_changed, labels, {.size = { 250.f, 0.f }}),
						{ .size = { 0.f, 23.f } }
					},
					new LabeledRow {
						new Text("rotation", labelStyle, { .size = { Grow, Grow } }),
						new Drag<f32, 3>("Rotation", reinterpret_cast<f32*>(&data.rotation), 1.f, -360.f, 360.f, &data.m_changed, labels, {.size = { 250.f, 0.f }}),
						{ .size = { 0.f, 23.f } }
					},
					new LabeledRow {
						new Text("scale", labelStyle, { .size = { Grow, Grow } }),
						new Drag<f32, 3>("Scale", reinterpret_cast<f32*>(&data.scale), 0.1f, -10.f, 10.f, &data.m_changed, labels, {.size = { 250.f, 0.f }}),
						{ .size = { 0.f, 23.f } }
					},
				}
			);
		}
	};
}
