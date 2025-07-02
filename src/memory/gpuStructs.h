//
// Created by radue on 6/17/2025.
//

#pragma once
#include "math/vector.h"

namespace Coral::GPU {
	namespace Light {
		struct Point {
			alignas(16) Math::Vector3<f32> position;
			alignas(16) Math::Vector3<f32> color;
			alignas(16) Math::Vector3<f32> attenuation;
			f32 radius;
		};

		struct Directional {
			alignas(16) Math::Vector3<f32> direction;
			alignas(16) Math::Vector3<f32> color;
			f32 intensity;
		};

		struct Spot {
			alignas(16) Math::Vector3<f32> position;
			alignas(16) Math::Vector3<f32> direction;
			alignas(16) Math::Vector3<f32> color;
			f32 innerAngle;
			f32 outerAngle;
			f32 intensity;
			f32 range;
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