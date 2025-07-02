//
// Created by radue on 5/7/2025.
//

#pragma once

#include "component.h"
#include "math/matrix.h"
#include "math/quaternion.h"
#include "math/transform.h"
#include "math/vector.h"

namespace Coral::Reef {
	class TransformTemplate;
}

namespace Coral::ECS {
    struct Transform final : Component {
    	friend class Coral::Reef::TransformTemplate;

    	Math::Vector3<f32> position;
    	Math::Vector3<f32> rotation;
    	Math::Vector3<f32> scale;

        explicit Transform(const Math::Vector3<f32>& position = {0.0f, 0.0f, 0.0f},
                           const Math::Vector3<f32>& rotation = {0.0f, 0.0f, 0.0f},
                           const Math::Vector3<f32>& scale = {1.0f, 1.0f, 1.0f})
            : position(position), rotation(rotation), scale(scale)
    	{
	        m_changed = true;
        	m_matrix = Math::Matrix4<f32>::Identity();
        }

    	Transform(const Transform&) = delete;
		Transform& operator=(const Transform&) = delete;

    	~Transform() override = default;

    	Transform Clone() const {
    		return Transform {
				position,
    			rotation,
    			scale,
    		};
    	}

        [[nodiscard]] Math::Matrix4<f32> Matrix() {
    		if (m_changed) {
    			const auto translation = Math::Translate(position);
    			const auto rotationMatrix = Math::Quaternion(Math::Radians(rotation)).ToMatrix();
    			const auto scaleMatrix = Math::Scale(scale);
    			m_matrix = scaleMatrix * rotationMatrix * translation;
    			m_changed = false;
    		}
    		return m_matrix;
        }

    	[[nodiscard]] bool Changed() const { return m_changed; }

    private:
    	bool m_changed { true };
    	Math::Matrix4<f32> m_matrix { Math::Matrix4<f32>::Identity() };
    };
}
