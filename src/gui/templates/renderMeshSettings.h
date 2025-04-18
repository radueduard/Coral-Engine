//
// Created by radue on 4/8/2025.
//

#pragma once
#include "IconsFontAwesome6.h"
#include "components/renderMesh.h"
#include "graphics/objects/material.h"
#include "gui/elements/button.h"
#include "gui/elements/column.h"
#include "gui/elements/labeledRow.h"
#include "gui/elements/text.h"

namespace GUI {
    class RenderMeshSettings final: public Template<Coral::RenderMesh> {
    public:
        RenderMeshSettings() = default;
        ~RenderMeshSettings() override = default;

        Element* Build(Coral::RenderMesh* data) override
        {
        	auto pairs = data->Targets();
        	std::vector<Element*> targets;
        	for (const auto& [mesh, material] : pairs)
			{
				const auto targetElement = new Column(
					{
						new GUI::LabeledRow(to_string(mesh->UUID()), new Button(mesh->Name(), [](){}, 20.f)),
						new GUI::LabeledRow(to_string(material->UUID()), new Button(material->Name(), [](){}, 20.f)),
					},
					10.f
				);
				targets.push_back(targetElement);
			}

	        return new GUI::Column(
				{
					new GUI::Text(ICON_FA_CUBE "    Render Target", GUI::Text::Style{ { 0.8f, 0.8f, 0.8f, 1.f }, 20.f, FontType::Black }),
					new GUI::Text("Targets:"),
					new GUI::Column(targets, 10.f),
				},
				10.f
			);
        }
    };
}
