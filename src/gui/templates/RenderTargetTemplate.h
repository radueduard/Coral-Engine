//
// Created by radue on 4/8/2025.
//

#pragma once
#include "IconsFontAwesome6.h"
#include "ecs/components/renderTarget.h"

#include "graphics/objects/mesh.h"
#include "graphics/objects/material.h"

#include "gui/reef.h"

namespace Coral::Reef {
    class RenderTargetTemplate final: public ReadWriteTemplate<ECS::RenderTarget> {
    public:
        RenderTargetTemplate() = default;
        ~RenderTargetTemplate() override = default;

        Element* Build(ECS::RenderTarget &data) override
        {
        	auto pairs = data.Targets();
        	std::vector<Element*> targets {
        		new Text(
					" " ICON_FA_CUBE "    Render Target",
					Text::Style{
						{ 0.8f, 0.8f, 0.8f, 1.f },
						20.f,
						FontType::Black
					},
        			{ .size = { 0.f, 20.f } }
        		),
				new Separator(),
        	};
        	for (const auto& [mesh, material] : pairs)
			{
				const auto targetElement = new Element(
					{
						.size = { Grow, 50.f },
					}, {
						new Element({},
							{
								new Text(mesh->Name(), Text::Style().withHorizontalAlignment(Text::HorizontalAlignment::Center)),
							}
						),
						new Element({},
							{
								new Text(material->Name(), Text::Style().withHorizontalAlignment(Text::HorizontalAlignment::Center)),
							}
						),
					}
				);
				targets.push_back(targetElement);
			}

	        return new Element(
				{
					.size = { Grow, Shrink },
					.padding = { 10.f, 10.f, 10.f, 10.f },
					.spacing = 10.f,
					.cornerRadius = 10.f,
					.backgroundColor = { 0.1f, 0.1f, 0.1f, 1.f },
					.direction = Axis::Vertical,
				},
				targets
	        );
        }
    };
}
