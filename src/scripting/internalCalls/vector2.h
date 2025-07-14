//
// Created by radue on 6/13/2025.
//

#pragma once

#include <mono/metadata/object-forward.h>
#include "math/vector.h"

// namespace Coral::Scripting {
// 	template<typename T> requires std::is_arithmetic_v<T>
// 	class Vector2 {
// 	public:
// 		static T get_x(MonoObject* instance) {
// 			return Math::Vector2<T>::LocalInstance(instance)->x;
// 		}
//
// 		static T get_y(MonoObject* instance) {
// 			return Math::Vector2<T>::LocalInstance(instance)->y;
// 		}
//
// 		static void set_x(MonoObject* instance, const T value) {
// 			Math::Vector2<T>::LocalInstance(instance)->x = value;
// 		}
//
// 		static void set_y(MonoObject* instance, const T value) {
// 			Math::Vector2<T>::LocalInstance(instance)->y = value;
// 		}
//
// 		static void ctor_no_args(MonoObject* obj) {
// 			auto* vec = new Math::Vector2<T>();
// 			vec->AddRemoteInstance(obj);
// 		}
//
// 		static void ctor_scalar(MonoObject* obj, const T scalar) {
// 			auto* vec = new Math::Vector2(scalar);
// 			vec->AddRemoteInstance(obj);
// 		}
//
// 		static void ctor_elements(MonoObject* obj, const T x, const T y) {
// 			auto* vec = new Math::Vector2 { x, y };
// 			vec->AddRemoteInstance(obj);
// 		}
//
// 		static MonoObject* op_addition(MonoObject* obj, MonoObject* other) {
// 			const auto& objLocal = *Math::Vector2<T>::LocalInstance(obj);
// 			const auto& otherLocal = *Math::Vector2<T>::LocalInstance(other);
// 			return **(new Math::Vector2(objLocal + otherLocal))->RemoteInstance();
// 		}
// 	};
//
// }