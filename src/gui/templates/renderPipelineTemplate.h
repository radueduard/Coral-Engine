//
// Created by radue on 11/25/2025.
//

#pragma once

#include <IconsFontAwesome6.h>
#include "graphics/pipeline.h"
#include "gui/elements/dropDown.h"
#include "gui/elements/labeledRow.h"
#include "gui/elements/separator.h"
#include "gui/elements/setSelection.h"
#include "template.h"


namespace Coral::Reef {
	class RenderPipelineTemplate : public ReadWriteTemplate<Graphics::Pipeline::Builder> {
	public:
		RenderPipelineTemplate() = default;
		~RenderPipelineTemplate() override = default;

		Element* Build(Graphics::Pipeline::Builder& data) override {
			const Text::Style labelStyle {
				.color = { 0.8f, 0.8f, 0.8f, 1.f },
				.fontSize = 15.f,
				.fontStyle = FontType::Bold,
			};

			return new Element({
					.size = { Grow, Shrink },
					.padding = { 10.f, 10.f, 10.f, 10.f },
					.cornerRadius = 10.f,
					.backgroundColor = { .1f, .1f, .1f, 1.f },
					.direction = Axis::Vertical,
				},
				{
					new Text(" " ICON_FA_PAINTBRUSH "   Graphics Pipeline",
						Text::Style{
							.color = Colors::grey[300],
							.fontSize = 20.f,
							.fontStyle = FontType::Black
						},
						{ .size = { 0.f, 20.f } }
					),
					new Separator(),
					new Text(
							"Rasterization State",
							Text::Style{
								.color = Colors::grey[300],
								.fontSize = 14.f,
								.fontStyle = FontType::Black,
								.verticalAlignment = Text::VerticalAlignment::Middle,
								.horizontalAlignment = Text::HorizontalAlignment::Center,
							},
					{ .size = { 0.f, 20.f } }
					),
					new Separator(),
					new LabeledRow {
						new Text("Depth Clamp Enable", labelStyle),
						new Checkbox(
							"Depth Clamp Enable",
							data.m_rasterizer.depthClampEnable,
							[&data](bool newValue) {
								data.m_rasterizer.depthClampEnable = newValue;
							},
							{ .size = { 23.f, 23.f } }
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("Depth Bias Clamp", labelStyle),
						new Drag<f32, 1>(
							"Depth Bias Clamp",
							&data.m_rasterizer.depthBiasClamp,
							0.1f,
							0.f,
							100.f,
							nullptr,
							{},
							{ .size = { 250.f, Grow } }
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("Rasterizer Discard Enable", labelStyle),
						new Checkbox(
							"Rasterizer Discard Enable",
							data.m_rasterizer.rasterizerDiscardEnable,
							[&data](bool newValue) {
								data.m_rasterizer.rasterizerDiscardEnable = newValue;
							},
							{ .size = { 23.f, 23.f } }
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("Polygon Mode", labelStyle),
						new DropDown("Polygon Mode",
							&data.m_rasterizer.polygonMode,
							{},
							{
								.size = { 250.f, 0.f },
							}),
						{
							.size = { 0.f, 23.f }
						}
					},
					new LabeledRow {
						new Text("Line Width", labelStyle),
						new Drag<f32, 1>(
							"Line Width",
							&data.m_rasterizer.lineWidth,
							1.f,
							1.f,
							20.f,
							nullptr,
							{},
							{ .size = { 250.f, Grow } }
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("Cull Mode", labelStyle),
						new FlagSetSelection(
							data.m_rasterizer.cullMode,
							{ .size = { 250.f, Grow } })
					},
					new LabeledRow {
						new Text("Front Face", labelStyle),
						new DropDown("Front Face",
							&data.m_rasterizer.frontFace,
							{},
							{ .size = { 250.f, 0.f } }
						),
						{ .size = { 0.f, 23.f } }
					},
					new LabeledRow {
						new Text("Depth Bias Enable", labelStyle),
						new Checkbox(
							"Depth Bias Enable",
							data.m_rasterizer.depthBiasEnable,
							[&data](bool newValue) {
								data.m_rasterizer.depthBiasEnable = newValue;
							},
							{ .size = { 23.f, 23.f } }
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("Depth Bias Constant Factor", labelStyle),
						new Drag<f32, 1>(
							"Depth Bias Constant Factor",
							&data.m_rasterizer.depthBiasConstantFactor,
							0.1f,
							0.f,
							100.f,
							nullptr,
							{},
							{ .size = { 250.f, Grow } }
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("Depth Bias Slope Factor", labelStyle),
						new Drag<f32, 1>(
							"Depth Bias Slope Factor",
							&data.m_rasterizer.depthBiasSlopeFactor,
							0.1f,
							0.f,
							100.f,
							nullptr,
							{},
							{ .size = { 250.f, Grow } }
						),
						{ .size = { Grow, 23.f } }
					},
					new Separator(),
					new Text(
						"Input Assembly State",
						Text::Style{
							.color = Colors::grey[300],
							.fontSize = 14.f,
							.fontStyle = FontType::Black,
							.verticalAlignment = Text::VerticalAlignment::Middle,
							.horizontalAlignment = Text::HorizontalAlignment::Center,
						},
						{ .size = { 0.f, 20.f } }
					),
					new Separator(),
					new LabeledRow {
						new Text("Primitive Topology", labelStyle),
						new DropDown("Primitive Topology",
							&data.m_inputAssembly.topology,
							{},
							{ .size = { 250.f, 0.f } }
						),
						{ .size = { 0.f, 23.f } }
					},
					new LabeledRow {
						new Text("Primitive Restart Enable", labelStyle),
						new Checkbox(
							"Primitive Restart Enable",
							data.m_inputAssembly.primitiveRestartEnable,
							[&data](bool newValue) {
								data.m_inputAssembly.primitiveRestartEnable = newValue;
							},
							{ .size = { 23.f, 23.f } }
						),
						{ .size = { Grow, 23.f } }
					},
					new Separator(),
					new Button(
						{
							.size = { Grow, 30.f },
							.cornerRadius = 5.f,
							.backgroundColor = { .2f, .2f, .2f, 1.f },
							.direction = Axis::Horizontal,
						},
						[&data] {
							data.m_shouldRebuild = true;
							return false;
						},
						{
							new Text("Reload", {
								.color = { 0.8f, 0.8f, 0.8f, 1.f },
								.fontSize = 15.f,
								.fontStyle = FontType::Bold,
								.verticalAlignment = Text::VerticalAlignment::Middle,
								.horizontalAlignment = Text::HorizontalAlignment::Center,
							}),
						}),
				}
			);
		}
	};
}