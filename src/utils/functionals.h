//
// Created by radue on 3/2/2025.
//


#pragma once

#include <functional>
#include <optional>
#include <ranges>

namespace Utils {
	template <class _Rng>
	concept input_range = std::ranges::range<_Rng> && std::input_iterator<std::ranges::iterator_t<_Rng>>;

	template <class _Fn, class _It>
	concept indirect_unary_predicate = std::indirectly_readable<_It> && std::copy_constructible<_Fn>
									   && std::predicate<_Fn&, std::_Indirect_value_t<_It>> && std::predicate<_Fn&, std::iter_reference_t<_It>>;

	template <input_range _Rng, class _Pj = std::identity,
				indirect_unary_predicate<std::projected<std::ranges::iterator_t<_Rng>, _Pj>> _Pr, class T = std::ranges::range_value_t<_Rng>>
	std::optional<T> FindIf(_Rng&& __rng, _Pr __pred) {
		std::optional<T> result = std::nullopt;
		auto __it = std::ranges::find_if(__rng, _Pr(__pred));
		if (__it != std::ranges::end(__rng)) {
			result = *__it;
		}
		return result;
	}
}