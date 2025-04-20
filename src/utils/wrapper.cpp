//
// Created by radue on 4/18/2025.
//

export module utils.wrapper;

import std;

namespace Coral::Utils {
	export template <typename T>
	class Wrapper {
	public:
		virtual ~Wrapper() = default;

		T operator*() const { return m_handle; }
		const T* operator->() const { return &m_handle; }

	protected:
		T m_handle;
	};
}