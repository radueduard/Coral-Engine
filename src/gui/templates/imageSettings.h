//
// Created by radue on 5/13/2025.
//

#pragma once

#include "gui/elements/drag.h"
#include "gui/elements/dropDown.h"
#include "gui/elements/inputField.h"
#include "gui/elements/labeledRow.h"
#include "gui/elements/setSelection.h"
#include "gui/elements/slider.h"
#include "memory/image.h"
#include "template.h"

namespace Coral::Reef {
	class ImageSettings final : public ReadWriteTemplate<Memory::Image::Builder> {
	public:
		Element* Build(Memory::Image::Builder& data) override {
			constexpr Text::Style labelStyle {
				.color = { 0.8f, 0.8f, 0.8f, 1.f },
				.fontSize = 15.f,
				.fontType = FontType::Bold,
			};

			return new Element(
				{
					.size = { Grow, Shrink },
					.spacing = 10.f,
					.cornerRadius = 10.f,
					.backgroundColor = { 0.1f, 0.1f, 0.1f, 1.f },
					.direction = Vertical,
				},
				{
					new LabeledRow(
						new Text(Text::Piece("Name:", labelStyle), { .size = { Shrink, Grow } }),
						new InputField("name", &data.m_name, { .size = { 250.f, Grow } }),
						{ .size = { Grow, 23.f }}
					),
					new LabeledRow(
						new Text(Text::Piece("Format", labelStyle), { .size = { Shrink, Grow } }),
						new DropDown("format", &data.m_format, {}, { .size = { 250.f, Grow } }),
						{ .size = { Grow, 23.f }}
					),
					new LabeledRow(
						new Text(Text::Piece("Extent", labelStyle), { .size = { Shrink, Grow } }),
						new Drag<u32, 3>("extent", &data.m_extent.x, 1, 1, 8192, std::nullopt, { .size = { 250.f, Grow } }),
						{ .size = { Grow, 23.f }}
					),
					new LabeledRow(
						new Text(Text::Piece("Mip Level Count", labelStyle), { .size = { Shrink, Grow } }),
						new Slider<u32, 1>("mipLevels", &data.m_mipLevels, 1, 14, 1, { .size = { 250.f, Grow } }),
						{ .size = { Grow, 23.f }}
					),
					new LabeledRow(
						new Text(Text::Piece("Layer Count", labelStyle), { .size = { Shrink, Grow } }),
						new Slider<u32, 1>("layer count", &data.m_layersCount, 1, 32, 1, { .size = { 250.f, Grow } }),
						{ .size = { Grow, 23.f }}
					),
					new LabeledRow(
						new Text(Text::Piece("Sample Count", labelStyle), { .size = { Shrink, Grow } }),
						new DropDown("sample count", &data.m_sampleCount, {}, { .size = { 250.f, Grow } }),
						{ .size = { Grow, 23.f }}
					),
					new LabeledRow(
						new Text(Text::Piece("Layout", labelStyle), { .size = { Shrink, Grow } }),
						new DropDown("layout", &data.m_layout, {}, { .size = { 250.f, Grow } }),
						{ .size = { Grow, 23.f }}
					),
					new LabeledRow(
						new Text(Text::Piece("Usage Flags", labelStyle), { .size = { Shrink, Grow } }),
						new SetSelection(&data.m_usageFlagsSet, {
							.size = { 250.f, Grow },
							.padding = { 10.f, 10.f, 10.f, 10.f },
							.spacing = 10.f,
							.cornerRadius = 5.f,
							.backgroundColor = { 0.f, 0.f, 0.f, 1.f },
						}),
						{ .size = { Grow, Shrink }}
					),
				}
			);
		}
	};
}