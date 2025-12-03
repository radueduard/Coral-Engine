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
								.fontSize = 20.f,
								.fontStyle = FontType::Black,
								.verticalAlignment = Text::VerticalAlignment::Middle,
								.horizontalAlignment = Text::HorizontalAlignment::Center,
							},
					{ .size = { 0.f, 20.f } }
					),
					new Separator(),
					new LabeledRow {
						new Text("Depth Clamp Enable"),
						new Checkbox(
							"Depth Clamp Enable",
							data.m_rasterizer.depthClampEnable,
							[&data](bool newValue) {
								data.m_rasterizer.depthClampEnable = newValue;
							},
							Checkbox::DefaultStyle()
								.withSize({ 23.f, Grow })
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("Depth Bias Clamp"),
						new Drag(
							"Depth Bias Clamp",
							&data.m_rasterizer.depthBiasClamp,
							0.1f,
							0.f,
							200.f,
							nullptr,
							{},
							DragDefaultStyle()
								.withSize({ 200.f, Grow })
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("Rasterizer Discard Enable"),
						new Checkbox(
							"Rasterizer Discard Enable",
							data.m_rasterizer.rasterizerDiscardEnable,
							[&data](bool newValue) {
								data.m_rasterizer.rasterizerDiscardEnable = newValue;
							},
							Checkbox::DefaultStyle()
								.withSize({ 23.f, Grow })
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("Polygon Mode"),
						new DropDown("Polygon Mode",
							&data.m_rasterizer.polygonMode,
							{},
							DropDownDefaultStyle()
								.withSize({ 200.f, Grow })
						),
						{
							.size = { 0.f, 23.f }
						}
					},
					new LabeledRow {
						new Text("Line Width"),
						new Drag(
							"Line Width",
							&data.m_rasterizer.lineWidth,
							1.f,
							1.f,
							20.f,
							nullptr,
							{},
							DragDefaultStyle()
								.withSize({ 200.f, Grow })
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("Cull Mode"),
						new FlagSetSelection(
							data.m_rasterizer.cullMode,
							{ .size = { 200.f, Grow } }),
							// { .size = { Grow, Grow } }
					},
					new LabeledRow {
						new Text("Front Face"),
						new DropDown("Front Face",
							&data.m_rasterizer.frontFace,
							{},
							DropDownDefaultStyle()
								.withSize({ 200.f, Grow })
						),
						{ .size = { 0.f, 23.f } }
					},
					new LabeledRow {
						new Text("Depth Bias Enable"),
						new Checkbox(
							"Depth Bias Enable",
							data.m_rasterizer.depthBiasEnable,
							[&data](bool newValue) {
								data.m_rasterizer.depthBiasEnable = newValue;
							},
							Checkbox::DefaultStyle()
								.withSize({ 23.f, Grow })
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("Depth Bias Constant Factor"),
						new Drag(
							"Depth Bias Constant Factor",
							&data.m_rasterizer.depthBiasConstantFactor,
							0.1f,
							0.f,
							200.f,
							nullptr,
							{},
							DragDefaultStyle()
								.withSize({ 200.f, Grow })
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("Depth Bias Slope Factor"),
						new Drag(
							"Depth Bias Slope Factor",
							&data.m_rasterizer.depthBiasSlopeFactor,
							0.1f,
							0.f,
							200.f,
							nullptr,
							{},
							DragDefaultStyle()
								.withSize({ 200.f, Grow })
						),
						{ .size = { Grow, 23.f } }
					},
					new Separator(),
					new Text(
						"Input Assembly State",
						Text::Style{
							.color = Colors::grey[300],
							.fontSize = 20.f,
							.fontStyle = FontType::Black,
							.verticalAlignment = Text::VerticalAlignment::Middle,
							.horizontalAlignment = Text::HorizontalAlignment::Center,
						},
						{ .size = { 0.f, 20.f } }
					),
					new Separator(),
					new LabeledRow {
						new Text("Primitive Topology"),
						new DropDown("Primitive Topology",
							&data.m_inputAssembly.topology,
							{},
							DropDownDefaultStyle()
								.withSize({ 200.f, Grow })
						),
						{ .size = { 0.f, 23.f } }
					},
					new LabeledRow {
						new Text("Primitive Restart Enable"),
						new Checkbox(
							"Primitive Restart Enable",
							data.m_inputAssembly.primitiveRestartEnable,
							[&data](bool newValue) {
								data.m_inputAssembly.primitiveRestartEnable = newValue;
							},
							Checkbox::DefaultStyle()
								.withSize({ 23.f, Grow })
						),
						{ .size = { Grow, 23.f } }
					},
					new Separator(),
					new Text(
						"Tessellation State",
						Text::Style{
							.color = Colors::grey[300],
							.fontSize = 20.f,
							.fontStyle = FontType::Black,
							.verticalAlignment = Text::VerticalAlignment::Middle,
							.horizontalAlignment = Text::HorizontalAlignment::Center,
						},
						{ .size = { Grow, 20.f } }
					),
					new Separator(),
					new LabeledRow {
						new Text("Patch Control Points"),
						new Drag<u32>(
							"Patch Control Points",
							&data.m_tessellation.patchControlPoints,
							1.f,
							1,
							32,
							nullptr,
							{},
							DragDefaultStyle()
								.withSize({ 200.f, Grow })
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