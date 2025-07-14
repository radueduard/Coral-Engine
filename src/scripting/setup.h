//
// Created by radue on 6/12/2025.
//

#pragma once
#include <mono/metadata/loader.h>


#include "assembly.h"
#include "internalCalls/input.h"
#include "internalCalls/vector2.h"
#include "internalCalls/vector3.h"
#include "math/vector.h"


namespace Coral::Scripting {
	inline void LinkClassesWithRemote() {
		const auto& assembly = Assembly::Get("Coral");

		const auto vector2Class = assembly.GetClass("Coral", "Vector2");
		const auto ivector2Class = assembly.GetClass("Coral", "IVector2");
		const auto vector3Class = assembly.GetClass("Coral", "Vector3");

		// Math::Vector2<f32>::s_class = vector2Class;
		// Math::Vector2<i32>::s_class = ivector2Class;
		// Math::Vector3<f32>::s_class = vector3Class;
	}

	inline void SetupInternalCalls() {

		// // Vector2
		// mono_add_internal_call("Coral.Vector2::get_x", Vector2<f32>::get_x);
		// mono_add_internal_call("Coral.Vector2::get_r", Vector2<f32>::get_x);
		// mono_add_internal_call("Coral.Vector2::set_x", Vector2<f32>::set_x);
		// mono_add_internal_call("Coral.Vector2::set_r", Vector2<f32>::set_x);
		// mono_add_internal_call("Coral.Vector2::get_y", Vector2<f32>::get_y);
		// mono_add_internal_call("Coral.Vector2::get_g", Vector2<f32>::get_y);
		// mono_add_internal_call("Coral.Vector2::set_y", Vector2<f32>::set_y);
		// mono_add_internal_call("Coral.Vector2::set_g", Vector2<f32>::set_y);
		//
		// mono_add_internal_call("Coral.Vector2::.ctor()", Vector2<f32>::ctor_no_args);
		// mono_add_internal_call("Coral.Vector2::.ctor(single)", Vector2<f32>::ctor_scalar);
		// mono_add_internal_call("Coral.Vector2::.ctor(single,single)", Vector2<f32>::ctor_elements);
		//
		// mono_add_internal_call("Coral.Vector2::op_Addition", Vector2<f32>::op_addition);
		//
		// // IVector2
		// mono_add_internal_call("Coral.IVector2::get_x", Vector2<i32>::get_x);
		// mono_add_internal_call("Coral.IVector2::get_r", Vector2<i32>::get_x);
		// mono_add_internal_call("Coral.IVector2::set_x", Vector2<i32>::set_x);
		// mono_add_internal_call("Coral.IVector2::set_r", Vector2<i32>::set_x);
		// mono_add_internal_call("Coral.IVector2::get_y", Vector2<i32>::get_y);
		// mono_add_internal_call("Coral.IVector2::get_g", Vector2<i32>::get_y);
		// mono_add_internal_call("Coral.IVector2::set_y", Vector2<i32>::set_y);
		// mono_add_internal_call("Coral.IVector2::set_g", Vector2<i32>::set_y);
		//
		// mono_add_internal_call("Coral.IVector2::.ctor()", Vector2<i32>::ctor_no_args);
		// mono_add_internal_call("Coral.IVector2::.ctor(single)", Vector2<i32>::ctor_scalar);
		// mono_add_internal_call("Coral.IVector2::.ctor(single,single)", Vector2<i32>::ctor_elements);
		//
		// mono_add_internal_call("Coral.IVector2::op_Addition", Vector2<i32>::op_addition);

		// Vector3
		// mono_add_internal_call("Coral.Vector3::get_x", Vector3::get_x);
		// mono_add_internal_call("Coral.Vector3::get_r", Vector3::get_x);
		// mono_add_internal_call("Coral.Vector3::set_x", Vector3::set_x);
		// mono_add_internal_call("Coral.Vector3::set_r", Vector3::set_x);
		// mono_add_internal_call("Coral.Vector3::get_y", Vector3::get_y);
		// mono_add_internal_call("Coral.Vector3::get_g", Vector3::get_y);
		// mono_add_internal_call("Coral.Vector3::set_y", Vector3::set_y);
		// mono_add_internal_call("Coral.Vector3::set_g", Vector3::set_y);
		// mono_add_internal_call("Coral.Vector3::get_z", Vector3::get_z);
		// mono_add_internal_call("Coral.Vector3::get_b", Vector3::get_z);
		// mono_add_internal_call("Coral.Vector3::set_z", Vector3::set_z);
		// mono_add_internal_call("Coral.Vector3::set_b", Vector3::set_z);
		//
		// mono_add_internal_call("Coral.Vector3::.ctor()", Vector3::ctor_no_args);
		// mono_add_internal_call("Coral.Vector3::.ctor(single)", Vector3::ctor_scalar);
		// mono_add_internal_call("Coral.Vector3::.ctor(single,single,single)", Vector3::ctor_elements);
		//
		// mono_add_internal_call("Coral.Vector3::op_Addition", Vector3::op_addition);

		// Input
		mono_add_internal_call("Coral.Input::GetKeyState", Coral::Input::GetKeyState);
    	mono_add_internal_call("Coral.Input::GetMouseButtonState", Coral::Input::GetMouseButtonState);
    	mono_add_internal_call("Coral.Input::IsKeyPressed", Coral::Input::IsKeyPressed);
    	mono_add_internal_call("Coral.Input::IsKeyHeld", Coral::Input::IsKeyHeld);
    	mono_add_internal_call("Coral.Input::IsKeyReleased", Coral::Input::IsKeyReleased);
    	mono_add_internal_call("Coral.Input::IsMouseButtonPressed", Coral::Input::IsMouseButtonPressed);
    	mono_add_internal_call("Coral.Input::IsMouseButtonHeld", Coral::Input::IsMouseButtonHeld);
    	mono_add_internal_call("Coral.Input::IsMouseButtonReleased", Coral::Input::IsMouseButtonReleased);

    	mono_add_internal_call("Coral.Input::GetMousePosition", Input::GetMousePosition);
    	mono_add_internal_call("Coral.Input::GetMousePositionDelta", Input::GetMousePositionDelta);
    	mono_add_internal_call("Coral.Input::GetMoseScrollDelta", Input::GetMouseScrollDelta);
	}
}