//
// Created by radue on 1/7/2026.
//

#include "component.h"

#include "context.h"
#include "ecs/scene.h"

Coral::ECS::Entity& Coral::ECS::Component::Entity() const {
	return Context::Scene().Entity(m_entity);
}
