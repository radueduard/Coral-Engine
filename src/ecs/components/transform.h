//
// Created by radue on 5/7/2025.
//

#pragma once

#include "math/matrix.h"
#include "math/quaternion.h"
#include "math/transform.h"
#include "math/vector.h"

namespace Coral::ECS {
    struct Transform {
        Math::Vector3<f32> position;
        Math::Vector3<f32> rotation;
        Math::Vector3<f32> scale;

        explicit Transform(const Math::Vector3<f32> position = {0.0f, 0.0f, 0.0f},
                           const Math::Vector3<f32> rotation = {0.0f, 0.0f, 0.0f},
                           const Math::Vector3<f32> scale = {1.0f, 1.0f, 1.0f})
            : position(position), rotation(rotation), scale(scale) {}

        [[nodiscard]] Math::Matrix4<f32> Matrix() const {
            const auto rotation = Math::Quaternion::FromEulerAngles(this->rotation);
            const auto translation = Math::Translate(position);
            const auto rotationMatrix = rotation.ToMatrix();
            const auto scaleMatrix = Math::Scale(scale);
            return scaleMatrix * rotationMatrix * translation;
        }
    };
}
