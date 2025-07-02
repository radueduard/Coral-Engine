//
// Created by radue on 5/10/2025.
//

#pragma once
#include <functional>

#include "button.h"
#include "element.h"
#include "text.h"

namespace Coral::Reef {
	class ContextMenu final : public Element {
	public:
		class Builder {
			friend class ContextMenu;

		public:
			explicit Builder() = default;

			Builder& AddItem(String text, std::function<bool()> onClick) {
				m_items.emplace_back(text, std::move(onClick));
				return *this;
			}

			ContextMenu* Build() { return new ContextMenu(this); }

		private:
			std::vector<std::pair<String, std::function<bool()>>> m_items;
		};

		bool Render() override {
			if (ImGui::BeginPopupContextItem()) {
				for (const auto& [text, onClick] : m_items) {
					if (ImGui::Selectable(text.c_str())) {
						if (onClick()) {
							ImGui::CloseCurrentPopup();
						}
					}
				}
				ImGui::EndPopup();
			}

			return false;
		}

	private:
		explicit ContextMenu(Builder* builder)
			: Element(Style { .size = {Shrink, Shrink}, .direction = Axis::Vertical }),
			  m_items(std::move(builder->m_items)) {}

		std::vector<std::pair<String, std::function<bool()>>> m_items;
	};
}
