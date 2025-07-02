//
// Created by radue on 6/12/2025.
//

#pragma once

#include <mono/metadata/object-forward.h>

#include "math/vector.h"
#include "scripting/object.h"

namespace Coral::Scripting {
	class Vector3 {
	public:
		static float get_x(MonoObject* instance) {
			return Math::Vector3<f32>::LocalInstance(instance)->x;
		}

		static float get_y(MonoObject* instance) {
			return Math::Vector3<f32>::LocalInstance(instance)->y;
		}

		static float get_z(MonoObject* instance) {
			return Math::Vector3<f32>::LocalInstance(instance)->z;
		}

		static void set_x(MonoObject* instance, const float value) {
			Math::Vector3<f32>::LocalInstance(instance)->x = value;
		}

		static void set_y(MonoObject* instance, const float value) {
			Math::Vector3<f32>::LocalInstance(instance)->y = value;
		}

		static void set_z(MonoObject* instance, const float value) {
			Math::Vector3<f32>::LocalInstance(instance)->z = value;
		}

		static void ctor_no_args(MonoObject* obj) {
			auto* vec = new Math::Vector3<f32>();
			vec->AddRemoteInstance(obj);
		}

		static void ctor_scalar(MonoObject* obj, const f32 scalar) {
			auto* local = new Math::Vector3(scalar);
			local->AddRemoteInstance(obj);
		}

		static void ctor_elements(MonoObject* obj, const f32 x, const f32 y, const f32 z) {
			const auto local = new Math::Vector3 { x, y, z };
			local->AddRemoteInstance(obj);
		}

		static MonoObject* op_addition(MonoObject* obj, MonoObject* other) {
			const auto& objLocal = *static_cast<Math::Vector3<f32>*>(Math::Vector3<f32>::LocalInstance(obj));
			const auto& otherLocal = *static_cast<Math::Vector3<f32>*>(Math::Vector3<f32>::LocalInstance(other));
			return **(new Math::Vector3(objLocal + otherLocal))->RemoteInstance();
		}

		// static MonoObject* op_subtraction(MonoObject* obj, MonoObject* other) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	const auto& otherLocal = *Math::Vector3<float>::LocalInstance(other);
		// 	return **(new Math::Vector3(local - otherLocal))->RemoteInstance();
		// }
		//
		// static MonoObject* op_unary_negation(MonoObject* obj) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	return **(new Math::Vector3(-local))->RemoteInstance();
		// }
		//
		// static MonoObject* op_multiplication(MonoObject* obj, MonoObject* other) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	const auto& otherLocal = *Math::Vector3<float>::LocalInstance(other);
		// 	return **(new Math::Vector3(local * otherLocal))->RemoteInstance();
		// }
		//
		// static MonoObject* op_division(MonoObject* obj, MonoObject* other) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	const auto& otherLocal = *Math::Vector3<float>::LocalInstance(other);
		// 	return **(new Math::Vector3(local / otherLocal))->RemoteInstance();
		// }
		//
		// static MonoObject* op_addition_float(MonoObject* obj, float scalar) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	return **(new Math::Vector3(local + scalar))->RemoteInstance();
		// }
		//
		// static MonoObject* op_subtraction_float(MonoObject* obj, float scalar) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	return **(new Math::Vector3(local - scalar))->RemoteInstance();
		// }
		//
		// static MonoObject* op_multiplication_float(MonoObject* obj, float scalar) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	return **(new Math::Vector3(local * scalar))->RemoteInstance();
		// }
		//
		// static MonoObject* op_division_float(MonoObject* obj, float scalar) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	return **(new Math::Vector3(local / scalar))->RemoteInstance();
		// }
		//
		// static float length(MonoObject* obj) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	return local.Length();
		// }
		//
		// static float squared_norm(MonoObject* obj) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	return local.SquaredNorm();
		// }
		//
		// static MonoObject* normalize(MonoObject* obj) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	const auto length = std::sqrt(local.x * local.x + local.y * local.y + local.z * local.z);
		// 	return **(new Math::Vector3(local.Normalize()))->RemoteInstance();
		// }
		//
		// static float dot(MonoObject* obj, MonoObject* other) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	const auto& otherLocal = *Math::Vector3<float>::LocalInstance(other);
		// 	return local.Dot(otherLocal);
		// }
		//
		// static MonoObject* cross(MonoObject* obj, MonoObject* other) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	const auto& otherLocal = *Math::Vector3<float>::LocalInstance(other);
		// 	return **(new Math::Vector3(local.Cross(otherLocal)))->RemoteInstance();
		// }
		//
		// static MonoObject* max(MonoObject* obj, MonoObject* other) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	const auto& otherLocal = *Math::Vector3<float>::LocalInstance(other);
		// 	return **(new Math::Vector3(Math::Vector3<float>::Max(local, otherLocal)))->RemoteInstance();
		// }
		//
		// static MonoObject* min(MonoObject* obj, MonoObject* other) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	const auto& otherLocal = *Math::Vector3<float>::LocalInstance(other);
		// 	return **(new Math::Vector3(Math::Vector3<float>::Min(local, otherLocal)))->RemoteInstance();
		// }
		//
		// static MonoObject* lerp(MonoObject* obj, MonoObject* other, float t) {
		// 	const auto& local = *Math::Vector3<float>::LocalInstance(obj);
		// 	const auto& otherLocal = *Math::Vector3<float>::LocalInstance(other);
		// 	return **(new Math::Vector3(Math::Vector3<float>::Lerp(local, otherLocal, t)))->RemoteInstance();
		// }
	};
}
