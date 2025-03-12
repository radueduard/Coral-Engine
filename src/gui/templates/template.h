//
// Created by radue on 2/22/2025.
//


#pragma once
#include "../elements/element.h"

namespace GUI {
	template <class T>
	class Template {
    public:
      	Template() = default;
		virtual ~Template() = default;

		Template(const Template&) = delete;
		Template& operator=(const Template&) = delete;

		virtual Element* Build(T* data) = 0;
    };
}
