//
// Created by radue on 12/16/2025.
//

#pragma once
#include "compute/program.h"
#include "template.h"

namespace Coral::Reef {
	class ComputeProgramTemplate : public Reef::ReadWriteTemplate<Compute::Program> {
	public:
		ComputeProgramTemplate() = default;
		~ComputeProgramTemplate() override = default;

		Element* Build(Compute::Program& data) override {
			const auto& shader = reinterpret_cast<const Shader::SlangShader&>(data.Shader());

			std::vector<Element*> sets;
			for (const auto& binding : data.Pipeline().DescriptorSetLayouts()) {
				std::vector<Element*> bindings;
				for (const auto& [bindingIndex, bindingInfo] : binding->Bindings()) {
					bindings.push_back(
						new LabeledRow {
							new Text(std::format("binding {}", bindingIndex)),
							new Text(
								std::format(
									"type: {}, count: {}",
									magic_enum::enum_name(bindingInfo.descriptorType),
									bindingInfo.descriptorCount
								),
								Text::Style(),
								Style { .size = { 200.f, Grow } }
							),
							{ .size = { Grow, 23.f } }
						}
					);
				}
				sets.push_back(
					new LabeledRow {
						new Text(
							std::format("Set {}", sets.size()),
							Text::Style()
								.withColor(Colors::grey[300])
								.withFontSize(18.f)
								.withFontStyle(FontType::Black),
							{ .size = { 40.f, Grow } }),
						new Element({
								.size = { Grow, Shrink },
								.padding = { 5.f, 5.f, 5.f, 5.f },
								.cornerRadius = 5.f,
								.backgroundColor = { .15f, .15f, .15f, 1.f },
								.direction = Axis::Vertical,
							},
							bindings
						),
						{ .size = { Grow, Shrink } }
					}
				);
			}

			return new Element({
					.size = { Grow, Shrink },
					.padding = { 10.f, 10.f, 10.f, 10.f },
					.cornerRadius = 10.f,
					.backgroundColor = { .1f, .1f, .1f, 1.f },
					.direction = Axis::Vertical,
				},{
					new Text(" " ICON_FA_GEAR " Compute Program",
					Text::Style()
						.withColor(Colors::grey[300])
						.withFontSize(20.f)
						.withFontStyle(FontType::Black),
						{ .size = { 0.f, 20.f } }
					),
					new Separator(),
					new Text(
						"Shader",
						Text::Style()
							.withColor(Colors::grey[300])
							.withFontSize(20.f)
							.withFontStyle(FontType::Black)
							.withVerticalAlignment(Text::VerticalAlignment::Middle)
							.withHorizontalAlignment(Text::HorizontalAlignment::Center),
						{ .size = { 0.f, 20.f } }
					),
					new Separator(),
					new LabeledRow {
						new Text("Module"),
						new Text(shader.Module() , Text::Style(),
							Style { .size = { 200.f, Grow } }
						),
						{ .size = { Grow, 23.f } }
					},
					new LabeledRow {
						new Text("Entry Point"),
						new Text(shader.EntryPoint(), Text::Style(),
							Style { .size = { 200.f, Grow } }
						),
						{ .size = { Grow, 23.f } }
					},
					new Separator(),
					new LabeledRow {
						new Text("Group Count"),
						new Drag<u32, 3>(
							"Group Count",
							{ &data.m_groupCount.x, &data.m_groupCount.y, &data.m_groupCount.z },
							1.f,
							{ 1, 1, 1 },
							Context::Runtime().PhysicalDevice().Properties().limits.maxComputeWorkGroupCount,
							nullptr,
							{},
							DragDefaultStyle()
								.withSize({ 200.f, Grow })
						),
						{ .size = { Grow, 23.f } }
					},
					new Separator(),
					new Text(
						"Descriptor Sets",
						Text::Style()
							.withColor(Colors::grey[300])
							.withFontSize(20.f)
							.withFontStyle(FontType::Black)
							.withVerticalAlignment(Text::VerticalAlignment::Middle)
							.withHorizontalAlignment(Text::HorizontalAlignment::Center),
						{ .size = { 0.f, 20.f } }
					),
					new Separator(),
					new Element({
							.size = { Grow, Shrink },
							.padding = { 5.f, 5.f, 5.f, 5.f },
							.cornerRadius = 5.f,
							.backgroundColor = { .15f, .15f, .15f, 1.f },
							.direction = Axis::Vertical,
						},
						sets
					),
					new Separator()
				}
			);
		}
	};
}
