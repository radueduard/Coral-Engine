//
// Created by radue on 5/13/2025.
//

#pragma once

#include "IconsFontAwesome6.h"
#include "button.h"
#include "dropDown.h"
#include "element.h"
#include "text.h"

namespace Coral::Reef {
	template <typename T> requires std::is_enum_v<T>
	class SetSelection final : public Element {
	public:
		explicit SetSelection(UnorderedSet<T>* set, const Style& style = Style())
			: Element(style), m_newValue(T{}), m_set(set)
		{
			m_axis = Axis::Vertical;
			Recreate();
		}
		~SetSelection() override = default;

		void Recreate() {
			m_newValue = T{};
			m_children.clear();
			for (const T& value : *m_set) {
				m_children.emplace_back(
					new Element(
						{
							.size = { Grow, 23.f },
							.spacing = m_spacing,
						},
						{
							new Text(String(magic_enum::enum_name(value).data())),
							new Element(),
							new Button(
								{ .size = { 23.f, 23.f }, .padding = { 8.f, 5.f, 5.f, 5.f }, .cornerRadius = 5.f },
								[&] () -> bool {
									m_set->erase(value);
									return true;
								},
								{ new Text(ICON_FA_MINUS) }
							),
						}
					)
				);
			}
			if (!SetFull()) {
				m_children.emplace_back(
					new Element(
						{
							.size = { Grow, 23.f },
							.spacing = m_spacing,
						},
						{
							new DropDown(magic_enum::enum_type_name<T>().data(), &m_newValue, *m_set),
							new Button(
								{ .size = { 23.f, 23.f }, .padding = { 8.f, 5.f, 5.f, 5.f }, .cornerRadius = 5.f },
								[&] () -> bool {
									if (m_newValue == T{}) {
										return false;
									}
									m_set->emplace(m_newValue);
									m_newValue = T{};
									return true;
								},
								{ new Text(ICON_FA_PLUS) }
							),
						}
					)
				);
			}
		}


	private:
		bool SetFull() const {
			const auto allValues = magic_enum::enum_entries<T>();
			for (auto [value, _] : allValues) {
				if (!m_set->contains(value)) {
					return false;
				}
			}
			return true;
		}

		void RecreateRequired() override {
			Recreate();
			Element::RecreateRequired();
		}

		T m_newValue;
		UnorderedSet<T>* m_set;
	};
}