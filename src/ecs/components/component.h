//
// Created by radue on 6/13/2025.
//

#pragma once

#include "entt/entity/entity.hpp"
#include "utils/types.h"

namespace Coral::ECS {
	class Entity;

	class Component {
	public:
		Component() = default;
		virtual ~Component() = default;

		Component(const Component&) = delete;
		Component& operator=(const Component&) = delete;

		[[nodiscard]] ECS::Entity& Entity() const;

		virtual void Setup() {}
		virtual void Update() {}

	protected:
		friend class Entity;
		void SetEntity(const entt::entity entity) {
			m_entity = entity;
		}

		entt::entity m_entity { entt::null };
	};
} // namespace Coral::ECS
