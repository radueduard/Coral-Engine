//
// Created by radue on 5/14/2025.
//

#pragma once

#include "gui/elements/drag.h"
#include "gui/elements/dropDown.h"
#include "gui/elements/inputField.h"
#include "gui/elements/labeledRow.h"
#include "gui/elements/setSelection.h"
#include "memory/image.h"
#include "template.h"

namespace Coral::Reef {
	class BufferSettings final : public ReadWriteTemplate<Memory::Buffer::Builder> {
		bool m_changed;
	public:
		Element* Build(Memory::Buffer::Builder& data) override {
			const Text::Style labelStyle {
				.color = { 0.8f, 0.8f, 0.8f, 1.f },
				.fontSize = 15.f,
				.fontStyle = FontType::Bold,
			};

			return new Element(
				{
					.size = { Grow, Shrink },
					.spacing = 10.f,
					.cornerRadius = 10.f,
					.backgroundColor = { 0.1f, 0.1f, 0.1f, 1.f },
					.direction = Axis::Vertical,
				},
				{
					new LabeledRow(
						new Text("Name:", labelStyle, { .size = { Shrink, Grow } }),
						new InputField("name", &data.m_name, { .size = { 250.f, Grow } }),
						{ .size = { Grow, 23.f }}
					),
					new LabeledRow(
						new Text("InstanceSize", labelStyle, { .size = { Shrink, Grow } }),
						new Drag<u32, 1>("instance size", &data.m_instanceSize, 1, 1, 256, &m_changed, std::nullopt, { .size = { 250.f, Grow } }),
						{ .size = { Grow, 23.f }}
					),
					new LabeledRow(
						new Text("InstanceCount", labelStyle, { .size = { Shrink, Grow } }),
						new Drag<u32, 1>("instance count", &data.m_instanceCount, 1, 1, 256, &m_changed, std::nullopt, { .size = { 250.f, Grow } }),
						{ .size = { Grow, 23.f }}
					),
					new LabeledRow(
						new Text("Usage Flags", labelStyle, { .size = { Shrink, Grow } }),
						new SetSelection(&data.m_usageFlagSet, {
							.size = { 250.f, Grow },
							.padding = { 10.f, 10.f, 10.f, 10.f },
							.spacing = 10.f,
							.cornerRadius = 5.f,
							.backgroundColor = { 0.f, 0.f, 0.f, 1.f },
						}),
						{ .size = { Grow, Shrink }}
					),
					new LabeledRow(
						new Text("Memory Property Flags", labelStyle, { .size = { Shrink, Grow } }),
						new SetSelection(&data.m_memoryPropertyFlagSet, {
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
