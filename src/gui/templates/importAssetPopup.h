//
// Created by radue on 6/25/2025.
//
#pragma once

#include "assets/importer.h"
#include "gui/elements/button.h"
#include "gui/elements/element.h"
#include "gui/elements/inputField.h"
#include "gui/elements/labeledRow.h"
#include "gui/elements/popup.h"
#include "gui/elements/separator.h"
#include "gui/elements/text.h"


namespace Coral::Reef {
	class ImportAssetPopup final: public Element {
	public:
		ImportAssetPopup() : Element(
			{},
			{
				new Popup(
					"##Import",
					{
						new Reef::Element(
							{},
							{
								new Reef::Element(),
								new Reef::Text("Import Asset", Text::Style {
									   .fontSize = 20.f,
									   .fontStyle = Reef::FontType::Black,
								   }, {
									   .size = {Reef::Shrink, 20.f},
								   }),
								new Reef::Element(),
							}
						),
						new Reef::Separator(),
						new Reef::LabeledRow(
							new Reef::Text(
								"Path",
								{
									.fontSize = 16.f,
									.fontStyle = Reef::FontType::Regular,
								},
								{.size = {Reef::Shrink, 20.f}}
							),
							new Element(
								{
									.spacing = 10.f,
								},
								{
									new Reef::InputField(
										  "##ImportAsset",
										  &m_assetPath,
										  {
											  .size = {300.f, Reef::Shrink},
											  .padding = {10.f, 10.f, 0.f, 0.f},
											  .cornerRadius = 5.f,
									}),
									new Reef::Button(
										 {
											 .size = {Reef::Shrink, 23.f},
											 .padding = {10.f, 10.f, 0.f, 0.f},
											 .cornerRadius = 5.f,
										 },
										 []() -> bool {
											 Context::GUIManager().GetPopup("##FileBrowser")->Open();
											 return true;
										 },
										 {
											 new Reef::Text(
											 	"Browse", {},
												{
													.size = { Reef::Shrink, Reef::Grow }
												}
											)
										 }
									),
								}
							)
						),
						new Reef::Element(
							{
								.size = {Reef::Grow, Reef::Shrink},
								.spacing = 10.f,
								.direction = Reef::Axis::Horizontal,
							},
							{
								new Reef::Element(),
								new Reef::Button(
									 {
										 .size = {Reef::Shrink, 23.f},
										 .padding = {10.f, 10.f, 0.f, 0.f},
										 .cornerRadius = 5.f,
									 },
									 []() -> bool {
										 Context::GUIManager().GetPopup("##Import")->Close();
										 return true;
									 },
									 {
									 	new Reef::Text("Cancel", {},
											 {
												.size = {Reef::Shrink, Reef::Grow}
											 }
										)
									 }
								),
								new Reef::Button(
									 {
										 .size = {Reef::Shrink, 23.f},
										 .padding = {10.f, 10.f, 0.f, 0.f},
										 .cornerRadius = 5.f,
									 },
									 [this]() -> bool {
										 Asset::Importer(m_assetPath).Import();
										 m_assetPath.clear();
										 m_assetPath.resize(256, '\0');
										Context::GUIManager().GetPopup("##Import")->Close();
										return true;
									 },
									 {
										new Reef::Text(
											"Import", {},
											{
												.size = {Reef::Shrink, Reef::Grow}
											}
										)
									 }
								),
								new Reef::Element(),
							}),
						},{
							.padding = {10.f, 10.f, 10.f, 10.f},
							.cornerRadius = 10.f,
							.direction = Reef::Axis::Vertical,
						}
					)
				}
			),
			m_assetPath(256, '\0') {}

	private:
		std::string m_assetPath;
	};
}