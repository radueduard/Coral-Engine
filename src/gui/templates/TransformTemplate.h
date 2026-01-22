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
						Text::Style()
							.withColor(Colors::grey[300])
							.withFontSize(20.f)
							.withFontStyle(FontType::Black)
							.withVerticalAlignment(Text::VerticalAlignment::Middle)
							.withHorizontalAlignment(Text::HorizontalAlignment::Left),
						{ .size = { Grow, 20.f } }
					),
					new Separator(),
					new LabeledRow {
						new Text("position"),
						new Drag<f32, 3>(
							"Position",
							{ &data.position.x, &data.position.y, &data.position.z },
							0.1f,
							{ -100.f, -100.f, -100.f },
							{ 100.f, 100.f, 100.f },
							&data.m_changed,
							labels,
							DragDefaultStyle()
								.withSize({ 250.f, Grow })
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("rotation"),
						new Drag<f32, 3>(
							"Rotation",
							{ &data.rotation.x, &data.rotation.y, &data.rotation.z },
							1.f,
							{ -180.f, -180.f, -180.f },
							{ 180.f, 180.f,	 180.f },
							&data.m_changed,
							labels,
							DragDefaultStyle()
								.withSize({ 250.f, Grow })
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("scale"),
						new Drag<f32, 3>(
							"Scale",
							{ &data.scale.x, &data.scale.y, &data.scale.z },
							0.1f,
							{ 0.01f, 0.01f, 0.01f },
							{ 10.f, 10.f, 10.f },
							&data.m_changed,
							labels,
							DragDefaultStyle()
								.withSize({ 250.f, Grow })
						),
						{ .size = { Grow, 23.f } }
					},
				}
			);
		}
	};
}
