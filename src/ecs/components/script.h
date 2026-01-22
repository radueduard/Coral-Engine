//
// Created by radue on 1/15/2026.
//

#pragma once

#include <functional>

#include "component.h"

#include "ecs/entity.h"

namespace Coral::ECS {
	class Script : public Component {
	public:
		explicit Script(std::function<void(ECS::Entity&)> setup,
						std::function<void(ECS::Entity&)> update)
			: m_Setup(std::move(setup)), m_Update(std::move(update)) {}
		~Script() override = default;

		void Setup() override {
			if (m_Setup) {
				m_Setup(Entity());
			}
		}

		void Update() override {
			if (m_Update) {
				m_Update(Entity());
			}
		}

	private:
		std::function<void(ECS::Entity&)> m_Setup;
		std::function<void(ECS::Entity&)> m_Update;
	};
}
