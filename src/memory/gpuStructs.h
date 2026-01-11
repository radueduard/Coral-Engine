//
// Created by radue on 6/17/2025.
//

#pragma once
#include "math/matrix.h"
#include "math/vector.h"

namespace Coral::GPU {
	namespace Light {
		struct Point {
			Math::Vector3<f32> position;
			f32 intensity;
			alignas(16) Math::Vector4f color;
			Math::Vector3<f32> attenuation;
			f32 range;
		};

		struct Directional {
			alignas(16) Math::Vector3<f32> direction;
			alignas(16) Math::Vector4f color;
			alignas(16) f32 intensity;
		};

		struct Spot {
			Math::Vector3<f32> position;
			f32 range;
			Math::Vector3<f32> direction;
			f32 intensity;
			alignas(16) Math::Vector4f color;
			alignas(16) Math::Vector3<f32> attenuation;
			f32 innerAngle;
			f32 outerAngle;
		};
	}

	struct Camera {
		alignas(64) Math::Matrix4<f32> view;
		alignas(64) Math::Matrix4<f32> projection;
		alignas(64) Math::Matrix4<f32> inverseView;
		alignas(64) Math::Matrix4<f32> inverseProjection;
	};

	struct Material {
		float alphaCutoff;
		uint32_t doubleSided;
		float roughnessFactor;
		float metallicFactor;
		alignas(16) glm::vec3 emissiveFactor;
		alignas(16) glm::vec4 baseColorFactor;
	};
}