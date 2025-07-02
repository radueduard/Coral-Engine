//
// Created by radue on 6/13/2025.
//

#pragma once

#include "entt/entity/entity.hpp"

namespace Coral::ECS {
	class Component {
	public:
		Component() = default;
		virtual ~Component() = default;

		Component(const Component&) = delete;
		Component& operator=(const Component&) = delete;

		[[nodiscard]] entt::entity Entity() const { return m_entity; }

	private:
		friend class Entity;
		void SetEntity(const entt::entity entity) {
			m_entity = entity;
		}

		entt::entity m_entity { entt::null };
	};
}
