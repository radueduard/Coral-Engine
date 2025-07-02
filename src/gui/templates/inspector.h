//
// Created by radue on 2/18/2025.
//

#pragma once

#include "ecs/entity.h"

#include "CameraTemplate.h"
#include "LightTemplate.h"
#include "RenderTargetTemplate.h"
#include "TransformTemplate.h"
#include "ecs/components/light.h"
#include "template.h"

#include "gui/reef.h"

namespace Coral::Reef {
	class EntityInspector final : public ReadWriteTemplate<ECS::Entity> {
	public:
		EntityInspector() {
			m_transformTemplate = std::make_unique<TransformTemplate>();
			m_cameraTemplate = std::make_unique<CameraTemplate>();
			m_renderTargetTemplate = std::make_unique<RenderTargetTemplate>();
			m_lightTemplate = std::make_unique<LightTemplate>();
		}

		Element *Build(ECS::Entity &data) override {
			std::vector<Element*> children {
				new LabeledRow(
					new Text(
						Text::Piece {"Name:", Text::Style { .fontSize = 15 }},
						{
							.size { Shrink, Grow },
							.padding { 5.f, 5.f, 0.f, 0.f },
						}
					),
					new InputField("Name", &data.Name(),
						{
							.size = { Grow, 23.f },
						}
					),
					{
						.size = { Grow, Shrink },
						.padding = { 10.f, 10.f, 10.f, 10.f },
						.cornerRadius = 10.f,
						.backgroundColor = { .1f, .1f, .1f, 1.f },
					}
				),
			};

			if (data.Has<ECS::Transform>()) {
				children.emplace_back(m_transformTemplate->Build(data.Get<ECS::Transform>()));
			}

			if (data.Has<ECS::Camera>()) {
				children.emplace_back(m_cameraTemplate->Build(data.Get<ECS::Camera>()));
			}

			if (data.Has<ECS::RenderTarget>()) {
				children.emplace_back(m_renderTargetTemplate->Build(data.Get<ECS::RenderTarget>()));
			}

			if (data.Has<ECS::Light>()) {
				children.emplace_back(m_lightTemplate->Build(data.Get<ECS::Light>()));
			}

			return new Element(
				{
					.spacing = 10.f,
					.direction = Axis::Vertical,
				},
				children
			);
		}
	private:
		std::unique_ptr<TransformTemplate> m_transformTemplate;
		std::unique_ptr<CameraTemplate> m_cameraTemplate;
		std::unique_ptr<RenderTargetTemplate> m_renderTargetTemplate;
		std::unique_ptr<LightTemplate> m_lightTemplate;
    };

	// class ShaderInspector final : public ReadWriteTemplate<Core::Shader> {
	// public:
	// 	Element *Build(Core::Shader &data) override {
	// 		Element* innerElement = new Column(
	// 			{
	// 				new Text(
	// 					ICON_FA_LOCATION_CROSSHAIRS "   Shader",
	// 					Text::Style{
	// 						{ 0.8f, 0.8f, 0.8f, 1.f },
	// 						20.f,
	// 						FontType::Black
	// 					}
	// 				),
	// 				new Text("Name: " + data.Name()),
	// 				new Text("Path: " + data.Path().string()),
	// 			},
	// 			10.f);
	//
	// 		return new Dockable(
	// 			ICON_FA_INFO "   Shader Inspector",
	// 			innerElement,
	// 			{ 10.f, 10.f }
	// 		);
	// 	}
	// };
}
