//
// Created by radue on 1/10/2026.
//

#include "entity.h"

#include "assets/manager.h"

#include "components/transform.h"
#include "components/camera.h"
#include "components/light.h"
#include "components/renderTarget.h"
#include "components/script.h"

namespace Coral::ECS {
	template<>
	const u16 TypeMask<Transform>() {
		return 0b1000'0000'0000'0000;
	}

	template<>
	const u16 TypeMask<Camera>() {
		return 0b0000'0000'0000'0010;
	}

	template<>
	const u16 TypeMask<Light>() {
		return 0b0000'0000'0000'0100;
	}

	template<>
	const u16 TypeMask<Script>() {
		return 0b0000'0000'0000'1000;
	}

	template<>
	const u16 TypeMask<RenderTarget>() {
		return 0b0000'0000'0001'0000;
	}

	Entity::Entity(String name) {
		auto& registry = Context::SceneManager().Registry();
		m_id = registry.create();
		registry.emplace<Entity*>(m_id, this);
		registry.emplace<Transform>(m_id);
		m_name = std::move(name);
		m_componentMask = TypeMask<Transform>();
	}

	Entity::~Entity() {
		if (auto& registry = Context::SceneManager().Registry()
			; registry.valid(m_id))
		{
			registry.remove<Entity*>(m_id);
			registry.remove<Transform>(m_id);
			registry.destroy(m_id);
		}
	}

	std::unique_ptr<Entity> Entity::Clone() const {
		auto& registry = Context::SceneManager().Registry();
		const auto& transform = registry.get<Transform>(m_id);

		auto newEntity = std::make_unique<Entity>(m_name);
		newEntity->m_id = registry.create();
		registry.emplace<Entity*>(newEntity->m_id, newEntity.get());
		registry.emplace<Transform>(newEntity->m_id, transform.position, transform.rotation, transform.scale);

		if (m_componentMask & TypeMask<RenderTarget>()) {
			const auto& renderTarget = registry.get<RenderTarget>(m_id);
			auto& newRenderTarget = newEntity->Add<RenderTarget>();
			for (const auto& [mesh, material] : renderTarget.Targets()) {
				newRenderTarget.Add(mesh, material);
			}
		}

		for (const auto& child : Children()) {
			auto clonedChild = child->Clone();
			newEntity->AddChild(std::move(clonedChild));
		}
		return newEntity;
	}

	void Entity::Update() const {
		auto& registry = Context::SceneManager().Registry();

		if (m_componentMask & TypeMask<Camera>()) {
			auto& camera = registry.get<Camera>(m_id);
			camera.Update();
		}

		if (m_componentMask & TypeMask<Light>()) {
			auto& light = registry.get<Light>(m_id);
			light.Update();
		}

		if (m_componentMask & TypeMask<Script>()) {
			auto& script = registry.get<Script>(m_id);
			script.Update();
		}

		if (m_componentMask & TypeMask<RenderTarget>()) {
			auto& renderTarget = registry.get<RenderTarget>(m_id);
			renderTarget.Update();
		}

		if (m_componentMask & TypeMask<Transform>()) {
			auto& transform = registry.get<Transform>(m_id);
			transform.Update();
		}
	}

	void Entity::AddEmpty() {
		static int counter = 0;
		auto child = std::make_unique<Entity>("Empty" + std::to_string(counter++));
		AddChild(std::move(child));
	}

	void Entity::AddCamera() {
		static int counter = 0;
		auto child = std::make_unique<Entity>("Camera" + std::to_string(counter++));
		child->Add<Camera>(Camera::CreateInfo {});
		AddChild(std::move(child));
	}

	Entity& Entity::AddLight(const LightType& type) {
		static int counter = 0;
		auto child = std::make_unique<Entity>("Light_" + std::to_string(counter++));

		switch (type) {
		case LightType::Point:
			child->Add<Light>(LightType::Point, false);
			return AddChild(std::move(child));
			break;
		case LightType::Directional:
			child->Add<Light>(LightType::Directional, true);
			return AddChild(std::move(child));
			break;
		case LightType::Spot:
			child->Add<Light>(LightType::Spot, true);
			return AddChild(std::move(child));
			break;
		}
		throw std::runtime_error("Unknown light type");
	}

	void Entity::AddCube() {
		static int counter = 0;
		auto child = std::make_unique<Entity>("Cube" + std::to_string(counter++));
		auto& renderTarget = child->Add<RenderTarget>();
		renderTarget.Add(Context::AssetManager().GetMesh(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000001")),
						 Context::AssetManager().GetMaterial(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000001")));
		AddChild(std::move(child));
	}

	void Entity::AddSphere() {
		static int counter = 0;
		auto child = std::make_unique<Entity>("Sphere" + std::to_string(counter++));
		auto& renderTarget = child->Add<RenderTarget>();
		renderTarget.Add(Context::AssetManager().GetMesh(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000002")),
						 Context::AssetManager().GetMaterial(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000001")));
		AddChild(std::move(child));
	}

	void Entity::AddCylinder() {
		static int counter = 0;
		auto child = std::make_unique<Entity>("Cylinder" + std::to_string(counter++));
		auto& renderTarget = child->Add<RenderTarget>();
		renderTarget.Add(Context::AssetManager().GetMesh(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000003")),
						 Context::AssetManager().GetMaterial(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000001")));
		AddChild(std::move(child));
	}

	void Entity::AddCone() {
		static int counter = 0;
		auto child = std::make_unique<Entity>("Cone" + std::to_string(counter++));
		auto& renderTarget = child->Add<RenderTarget>();
		renderTarget.Add(Context::AssetManager().GetMesh(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000005")),
						 Context::AssetManager().GetMaterial(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000001")));
		AddChild(std::move(child));
	}

	void Entity::AddPrism() {
		static int counter = 0;
		auto child = std::make_unique<Entity>("Prism" + std::to_string(counter++));
		auto& renderTarget = child->Add<RenderTarget>();
		renderTarget.Add(Context::AssetManager().GetMesh(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000004")),
						 Context::AssetManager().GetMaterial(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000001")));
		AddChild(std::move(child));
	}

	void Entity::Duplicate() {
		auto clonedEntity = Clone();
		clonedEntity->Name() += "_copy";

		if (m_parent) {
			m_parent->AddChild(std::move(clonedEntity));
		}
	}
}
