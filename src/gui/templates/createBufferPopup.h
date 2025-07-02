//
// Created by radue on 6/25/2025.
//

#pragma once

#include "assets/importer.h"
#include "gui/elements/button.h"
#include "gui/elements/element.h"
#include "gui/elements/popup.h"
#include "gui/elements/separator.h"
#include "gui/elements/text.h"
#include "imageSettings.h"


namespace Coral::Reef {
	class CreateImagePopup final: public Element {
	public:
		CreateImagePopup() {
			imageSettings = std::make_unique<ImageSettings>();
			m_children.emplace_back(std::make_unique<Element>(
				Style {},
				std::vector<Element*> {
					new Reef::Popup(
						"##CreateImage",
						{
							new Reef::Element(
								{},
								{
									new Reef::Element(),
									new Reef::Text(
										Reef::Text::Piece(
											"Create Image",
											{
											.fontSize = 20.f,
											.fontType = Reef::FontType::Black,
											}
										),
										{
											.size = {Reef::Shrink, 20.f},
										}
									),
									new Reef::Element(),
								}
							),
							new Reef::Separator(),
							imageSettings->Build(imageBuilder),
							new Reef::Element(
								{
									.size = {Reef::Grow, Reef::Shrink},
									.spacing = 10.f,
								},
								{
									new Reef::Element(),
									new Reef::Button(
										{
											.size = {Reef::Shrink, 23.f},
											.padding = {10.f, 10.f, 0.f, 0.f},
											.cornerRadius = 5.f,
										},
										[this]() -> bool {
											Reef::GlobalManager().GetPopup("##CreateImage")->Close();
											imageBuilder = Memory::Image::Builder();
											return true;
										},
										{
											new Reef::Text(
												Reef::Text::Piece("Cancel"),
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
											Reef::GlobalManager().GetPopup("##CreateImage")->Close();
											auto image = imageBuilder.Build();
											imageBuilder = Memory::Image::Builder();
											return true;
										},
										{
											new Reef::Text(
												Reef::Text::Piece("Submit"),
												{
													.size = {Reef::Shrink, Reef::Grow}
												}
											)
										}
									),
									new Reef::Element(),
								}
							)
						},
						{
						.padding = {10.f, 10.f, 10.f, 10.f},
						.cornerRadius = 10.f,
						.direction = Axis::Vertical,
						}
					)
				}));
		}

	private:
		std::unique_ptr<ImageSettings> imageSettings = nullptr;
		Memory::Image::Builder imageBuilder {};
	};

	class CreateBufferPopup final: public Element {
	public:
		CreateBufferPopup() {
			bufferSettings = std::make_unique<BufferSettings>();
			m_children.emplace_back(std::make_unique<Element>(
				Style {},
				std::vector<Element*> {
					new Reef::Popup(
						"##CreateBuffer",
						{
							new Reef::Element(
								{},
								{
									new Reef::Element(),
									new Reef::Text(
										Reef::Text::Piece(
											"Create Buffer",
											{
												.fontSize = 20.f,
												.fontType = Reef::FontType::Black,
											}
										),
										{
											.size = {Reef::Shrink, 20.f},
										}
									),
									new Reef::Element(),
								}
							),
							new Reef::Separator(),
							bufferSettings->Build(bufferBuilder),
							new Reef::Element(
								{
									.size = {Reef::Grow, Reef::Shrink},
									.spacing = 10.f,
								},
								{
									new Reef::Element(),
									new Reef::Button(
										{
											.size = { Reef::Shrink, 23.f },
											.padding = {10.f, 10.f, 0.f, 0.f},
											.cornerRadius = 5.f,
										},
										[this]() -> bool {
											Reef::GlobalManager().GetPopup("##CreateBuffer")->Close();
											bufferBuilder = Memory::Buffer::Builder();
											return true;
										},
										{
											new Reef::Text(
												Reef::Text::Piece("Cancel"),
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
											Reef::GlobalManager().GetPopup("##CreateBuffer")->Close();
											auto buffer = bufferBuilder.Build();
											bufferBuilder = Memory::Buffer::Builder();
											return true;
										},
										{
											new Reef::Text(
												Reef::Text::Piece("Submit"),
												{
													.size = {Reef::Shrink, Reef::Grow}
												}
											)
										}
									),
									new Reef::Element(),
								}
							)
						},
						{
							.padding = {10.f, 10.f, 10.f, 10.f},
							.cornerRadius = 10.f,
							.direction = Reef::Axis::Vertical,
						}
					)
				}
			));
		}
		std::unique_ptr<BufferSettings> bufferSettings = nullptr;
		Memory::Buffer::Builder bufferBuilder {};
	};
}