//
// Created by radue on 5/13/2025.
//

#pragma once

#include "IconsFontAwesome6.h"
#include "button.h"
#include "checkbox.h"
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
			m_style.direction = Axis::Vertical;
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
							.spacing = m_style.spacing,
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
							.spacing = m_style.spacing,
						},
						{
							new DropDown(magic_enum::enum_type_name<T>().data(), &m_newValue, *m_set),
							new Button(
								{
									.size = { 23.f, 23.f },
									.padding = { 8.f, 5.f, 5.f, 5.f },
									.cornerRadius = 5.f
								},
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

		bool RecreateRequired() override {
			Recreate();
			return Element::RecreateRequired();
		}

		T m_newValue;
		UnorderedSet<T>* m_set;
	};

	template <typename T> requires std::is_enum_v<T>
	std::set<T> GetMinimalFlagSet(std::set<T> flags) {
		for (const auto& value : flags) {
			for (const auto& otherValue : flags) {
				if (value == otherValue) {
					continue;
				}
				if ((value & otherValue) == otherValue && (value | otherValue) == value) {
					if (flags.contains(value)) {
						flags.erase(value);
						return GetMinimalFlagSet(flags);
					}
				}
			}
		}
		return flags;
	}

	template <typename T>
	class FlagSetSelection final : public Element {
	public:
		FlagSetSelection(vk::Flags<T>& flags, const Style& style = Style())
			: Element(style), m_flags(flags)
		{
			m_style.direction = Axis::Vertical;

			auto allValues = magic_enum::enum_entries<T>();

			std::set<T> minFlags;
			for (const auto& [value, _] : allValues) {
				minFlags.insert(value);
			}
			minFlags.erase(T{0});
			minFlags = GetMinimalFlagSet(minFlags);

			for (const auto& value : minFlags) {
				m_values[value] = (m_flags & value) == value;
			}
			Recreate();
		}

		void Recreate() {
			m_children.clear();
			for (auto& [value, enabled] : m_values) {
				m_children.emplace_back(
					new LabeledRow(
						new Text(
							String(magic_enum::enum_name(value).data()),
							{
								.verticalAlignment = Text::VerticalAlignment::Middle,
								.horizontalAlignment = Text::HorizontalAlignment::Left,
							}
						),
						new Checkbox(
							magic_enum::enum_name(value).data(),
							enabled,
							[&enabled, this, value] (bool newValue) {
								enabled = newValue;
								if (newValue) {
									m_flags |= value;
								} else {
									m_flags &= ~value;
								}
							}, {
								.size = { 23.f, 23.f },
								.cornerRadius = 5.f
							}
						), {
							.size = { Grow, 23.f },
							.padding = { 5.f, 0.f, 0.f, 0.f },
							.spacing = m_style.spacing,
						}
					)
				);
			}
		}

	private:
		// bool RecreateRequired() override {
		// 	Recreate();
		// 	return true;
		// }

		vk::Flags<T>& m_flags;
		std::unordered_map<T, bool> m_values {};
	};
}