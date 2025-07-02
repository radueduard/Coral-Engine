//
// Created by radue on 6/25/2025.
//

#include "prefab.h"

#include <boost/unordered/unordered_map.hpp>

#include "ecs/components/RenderTarget.h"
#include "ecs/components/camera.h"
#include "ecs/components/light.h"
#include "ecs/entity.h"
#include "gui/elements/popup.h"
#include "manager.h"

namespace Coral::ECS {
	struct Light;
}
Coral::Asset::Prefab::Prefab(std::string name, nlohmann::json metadata) :
	m_name(std::move(name)), m_metadata(std::move(metadata)) {}
void Coral::Asset::Prefab::Load() const {
	boost::unordered_map<boost::uuids::uuid, ECS::Entity*> objectMap;
	for (const auto& [uuid, objectData] : m_metadata["objects"].items()) {
		const auto object = new ECS::Entity(objectData["name"].get<std::string>());
		auto& transform = object->Get<ECS::Transform>();
		transform.position = {objectData["transform"]["position"][0].get<float>(),
							  objectData["transform"]["position"][1].get<float>(),
							  objectData["transform"]["position"][2].get<float>()};
		transform.rotation = {objectData["transform"]["rotation"][0].get<float>(),
							  objectData["transform"]["rotation"][1].get<float>(),
							  objectData["transform"]["rotation"][2].get<float>()};
		transform.scale = {objectData["transform"]["scale"][0].get<float>(),
						   objectData["transform"]["scale"][1].get<float>(),
						   objectData["transform"]["scale"][2].get<float>()};
		objectMap[_stringToUuid(uuid)] = object;
	}

	ECS::Entity* root = nullptr;
	for (const auto& [uuid, objectData] : m_metadata["objects"].items()) {
		const auto childUUID = _stringToUuid(uuid);
		const auto parentUUID = _stringToUuid(objectData["parent"].get<std::string>());

		auto* child = objectMap[childUUID];
		if (objectData.contains("meshes") && !objectData["meshes"].empty()) {
			auto& renderTarget = child->Add<ECS::RenderTarget>();
			for (const auto& meshData : objectData["meshes"]) {
				const auto meshUUID = _stringToUuid(meshData["mesh"].get<std::string>());
				const auto materialUUID = _stringToUuid(meshData["material"].get<std::string>());
				renderTarget.Add(Manager::Get().GetMesh(meshUUID),
								 Manager::Get().GetMaterial(materialUUID));
			}
		}
		if (objectData.contains("lights")) {
			for (const auto& lightUUID : objectData["lights"]) {
				const auto& lightData = m_metadata["lights"][lightUUID];
				ECS::Light::Type type;
				ECS::Light::Data data;
				if (lightData["type"].get<std::string>() == "aiLightSource_POINT") {
					type = ECS::Light::Type::Point;
					data.point.color =  {1.0f, .8f, 0.6f};
					data.point.attenuation = { 1.0f, 0.09f, 0.032f };
					data.point.range = 3.f;
				}
				else if (lightData["type"].get<std::string>() == "aiLightSource_DIRECTIONAL") {
					type = ECS::Light::Type::Directional;
					data.directional.color = {1.0f, 1.0f, 1.0f}; // Default color for directional light
					data.directional.direction = {lightData["direction"][0].get<float>(),
												 lightData["direction"][1].get<float>(),
												 lightData["direction"][2].get<float>()};
					data.directional.intensity = lightData["intensity"].get<float>();
				}
				// TODO: Handle spot lights when implemented
				// case "aiLightSource_SPOT":
				// 	type = ECS::Light::Type::Spot;
				// 	break;
				else {
					throw std::runtime_error("Unknown light type: " + lightData["type"].get<std::string>());
				}
				child->Add<ECS::Light>(type, data);
			}
		}

		if (objectData.contains("cameras")) {
			for (const auto& cameraUUID : objectData["cameras"]) {
				const auto& cameraData = m_metadata["cameras"][cameraUUID];
				ECS::Camera::Type cameraType = magic_enum::enum_cast<ECS::Camera::Type>(cameraData["type"].get<std::string>()).value();
				ECS::Camera::CreateInfo cameraCreateInfo;
				cameraCreateInfo.projectionData = ECS::Camera::ProjectionData(cameraType);
				switch (cameraType) {
					case ECS::Camera::Type::Perspective: {
						cameraCreateInfo.projectionData.data.perspective.fov = cameraData["fov"].get<float>();
						cameraCreateInfo.projectionData.data.perspective.near = cameraData["near"].get<float>();
						cameraCreateInfo.projectionData.data.perspective.far = cameraData["far"].get<float>();
					} break;
					case ECS::Camera::Type::Orthographic: {
						cameraCreateInfo.projectionData.data.orthographic.left = cameraData["left"].get<float>();
						cameraCreateInfo.projectionData.data.orthographic.right = cameraData["right"].get<float>();
						cameraCreateInfo.projectionData.data.orthographic.bottom = cameraData["bottom"].get<float>();
						cameraCreateInfo.projectionData.data.orthographic.top = cameraData["top"].get<float>();
						cameraCreateInfo.projectionData.data.orthographic.near = cameraData["near"].get<float>();
						cameraCreateInfo.projectionData.data.orthographic.far = cameraData["far"].get<float>();
					} break;
				}
				child->Add<ECS::Camera>(cameraCreateInfo);
			}
		}

		if (parentUUID != boost::uuids::nil_uuid()) {
			objectMap[parentUUID]->AddChild(child);
		}
		else {
			root = objectMap[childUUID];
		}
	}
	ECS::SceneManager::Get().GetLoadedScene().Root().AddChild(std::unique_ptr<ECS::Entity>(root));
}
