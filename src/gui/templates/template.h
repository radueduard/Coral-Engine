//
// Created by radue on 2/22/2025.
//


#pragma once
#include "gui/elements/element.h"

namespace Coral::Reef {
	class Template {
    public:
      	Template() = default;
		virtual ~Template() = default;

		Template(const Template&) = delete;
		Template& operator=(const Template&) = delete;
    };

	template <class T>
	class ReadOnlyTemplate : public Template {
		virtual Element* Build(const T& data) = 0;
	};

	template <class T>
	class ReadWriteTemplate : public Template {
		virtual Element* Build(T& data) = 0;
	};
}
