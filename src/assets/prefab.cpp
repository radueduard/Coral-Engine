//
// Created by radue on 6/25/2025.
//

#include "prefab.h"

#include <boost/unordered/unordered_map.hpp>
#include <magic_enum/magic_enum.hpp>

#include "ecs/components/camera.h"
#include "ecs/components/light.h"
#include "ecs/components/renderTarget.h"
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
				renderTarget.Add(Context::AssetManager().GetMesh(meshUUID),
								 Context::AssetManager().GetMaterial(materialUUID));
			}
		}
		if (objectData.contains("lights")) {
			for (const auto& lightUUID : objectData["lights"]) {
				const auto& lightData = m_metadata["lights"][lightUUID];
				ECS::LightType type;
				if (lightData["type"].get<std::string>() == "aiLightSource_POINT") {
					type = ECS::LightType::Point;
					auto& light = child->Add<ECS::Light>(type, false);


					light.color = {1.0f, .8f, 0.6f};
					light.attenuation = { 1.0f, 0.09f, 0.032f };
					light.range = 3.f;
				}
				else if (lightData["type"].get<std::string>() == "aiLightSource_DIRECTIONAL") {
					type = ECS::LightType::Directional;
					auto& light = child->Add<ECS::Light>(type, true);
					auto& transform = child->Get<ECS::Transform>();
					transform.rotation = {lightData["rotation"][0].get<float>(),
										  lightData["rotation"][1].get<float>(),
										  lightData["rotation"][2].get<float>()};

					light.color = {1.0f, 1.0f, 1.0f}; // Default color for directional light
					light.intensity = lightData["intensity"].get<float>();
				} else if (lightData["type"].get<std::string>() == "aiLightSource_AMBIENT") {
					// Ambient light is usually not represented as an entity in ECS.
					// It might be a global setting. So we can skip adding a light component here.
					continue;
				} else if (lightData["type"].get<std::string>() == "aiLightSource_AREA") {
					// Area lights are not supported in this ECS implementation.
					continue;
				} else if (lightData["type"].get<std::string>() == "aiLightSource_SPOT") {
					type = ECS::LightType::Spot;
					auto& light = child->Add<ECS::Light>(type, true);
					auto& transform = child->Get<ECS::Transform>();
					transform.rotation = {lightData["rotation"][0].get<float>(),
										  lightData["rotation"][1].get<float>(),
										  lightData["rotation"][2].get<float>()};

					light.color = {1.0f, 1.0f, 1.0f}; // Default color for spot light
					light.intensity = lightData["intensity"].get<float>();
					light.innerAngle = lightData["innerAngle"].get<float>();
					light.outerAngle = lightData["outerAngle"].get<float>();
					light.range = lightData["range"].get<float>();
				} else {
					throw std::runtime_error("Unknown light type: " + lightData["type"].get<std::string>());
				}
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
	Context::Scene().Root().AddChild(std::unique_ptr<ECS::Entity>(root));
}
