//
// Created by radue on 11/5/2024.
//

#include "importer.h"

#include <boost/uuid/uuid_io.hpp>
#include <execution>
#include <thread>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <stack>
#include <stb_image.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>
#include <glm/gtc/quaternion.hpp>

#include "../ecs/components/RenderTarget.h"
#include "../ecs/scene.h"
#include "ecs/Entity.h"
#include "ecs/components/camera.h"
#include "graphics/objects/material.h"
#include "graphics/objects/mesh.h"
#include "graphics/objects/textureArray.h"
#include "gui/elements/popup.h"
#include "manager.h"
#include "prefab.h"

namespace Coral::ECS {
    struct Transform;
}

namespace Coral::Asset {
    Importer::Importer(const std::string &path) {
        m_path = path.substr(0, path.find_last_of('/'));
        m_name = path.substr(path.find_last_of('/') + 1);

        constexpr auto flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenBoundingBoxes;
        m_scene = _importer.ReadFile(path, flags);

        if (!m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode) {
            std::cerr << "Failed to load scene: " << _importer.GetErrorString() << std::endl;
        }

        m_textureSize = 0;
        for (uint32_t i = 0; i < m_scene->mNumMaterials; i++) {
            const auto material = m_scene->mMaterials[i];
            aiString path;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
                int width, height;
                stbi_info((m_path + "/" + path.C_Str()).c_str(), &width, &height, nullptr);
                m_textureSize = std::max(m_textureSize, std::max(static_cast<uint32_t>(width), static_cast<uint32_t>(height)));
            }
        }
    }

    void Importer::Import() {
        static auto generator = boost::uuids::random_generator_mt19937();
        m_metadata["meshes"] = nlohmann::json::object();
        m_metadata["materials"] = nlohmann::json::object();
        m_metadata["textures"] = nlohmann::json::object();
    	m_metadata["lights"] = nlohmann::json::object();
    	m_metadata["cameras"] = nlohmann::json::object();
        m_metadata["objects"] = nlohmann::json::object();

        std::stack<std::pair<const aiNode*, std::string>> nodes;
        nodes.emplace(m_scene->mRootNode, boost::uuids::to_string(boost::uuids::nil_uuid()));
        while (!nodes.empty()) {
            const auto [node, parent] = nodes.top();
            nodes.pop();

            aiVector3D position;
            aiQuaternion rotation;
            aiVector3D scale;

        	aiMatrix4x4 transform = node->mTransformation;
        	aiNode* parentNode = node->mParent;
        	while (parentNode) {
        		transform = parentNode->mTransformation.Inverse() * transform;
        		parentNode = parentNode->mParent;
        	}
        	if (transform.IsIdentity()) {
				position = aiVector3D(0, 0, 0);
				rotation = aiQuaternion(1, 0, 0, 0);
				scale = aiVector3D(1, 1, 1);
			} else {
				transform.Decompose(scale, rotation, position);
			}
            glm::quat rotationQuaternion = {rotation.w, rotation.x, rotation.y, rotation.z};
            glm::vec3 positionVector = {position.x, position.y, position.z};
            glm::vec3 rotationVector = glm::degrees(glm::eulerAngles(rotationQuaternion));
            glm::vec3 scaleVector = {scale.x, scale.y, scale.z};

            auto objectUUID = boost::uuids::to_string(generator());
            auto& objectData = m_metadata["objects"][objectUUID];
            objectData["parent"] = parent;
            objectData["name"] = node->mName.C_Str();
            objectData["transform"] = nlohmann::json::object();
            objectData["transform"]["position"] = { positionVector.x, positionVector.y, positionVector.z };
            objectData["transform"]["rotation"] = { rotationVector.x, rotationVector.y, rotationVector.z };
            objectData["transform"]["scale"] = { scaleVector.x, scaleVector.y, scaleVector.z };
            objectData["meshes"] = nlohmann::json::array();

            for (uint32_t i = 0; i < node->mNumMeshes; i++) {
                const auto meshId = node->mMeshes[i];
                const auto materialIndex = m_scene->mMeshes[meshId]->mMaterialIndex;

                std::string meshUUID;
                bool meshExists = false;
                for (const auto& mesh : m_metadata["meshes"].items()) {
                    if (mesh.value()["id"] == meshId) {
                        meshExists = true;
                        meshUUID = mesh.key();
                        break;
                    }
                }
                if (!meshExists) {
                    meshUUID = boost::uuids::to_string(generator());
                    auto& meshData = m_metadata["meshes"][meshUUID];
                    meshData["id"] = meshId;
                    meshData["name"] = m_scene->mMeshes[meshId]->mName.C_Str();
                }

                std::string materialUUID;
                bool materialExists = false;
                for (const auto& material : m_metadata["materials"].items()) {
                    if (material.value()["id"] == materialIndex) {
                        materialExists = true;
                        materialUUID = material.key();
                        break;
                    }
                }
                if (!materialExists) {
                    materialUUID = boost::uuids::to_string(generator());
                    auto& materialData = m_metadata["materials"][materialUUID];
                    materialData["id"] = materialIndex;

                    const auto material = m_scene->mMaterials[materialIndex];
                    aiString name;
                    material->Get(AI_MATKEY_NAME, name);
                    materialData["name"] = name.C_Str();

                    ai_real alphaCutoff;
                    material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff);
                    materialData["alphaCutoff"] = alphaCutoff;

                    ai_int doubleSided;
                    material->Get(AI_MATKEY_TWOSIDED, doubleSided);
                    materialData["doubleSided"] = doubleSided;

                    ai_real roughnessFactor;
                    material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor);
                    materialData["roughnessFactor"] = roughnessFactor;

                    ai_real metallicFactor;
                    material->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor);
                    materialData["metallicFactor"] = metallicFactor;

                    aiColor3D emissiveFactor;
                    material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveFactor);
                    materialData["emissiveFactor"] = {emissiveFactor.r, emissiveFactor.g, emissiveFactor.b};

                    aiColor4D baseColorFactor;
                    material->Get(AI_MATKEY_BASE_COLOR, baseColorFactor);
                    materialData["baseColorFactor"] = {baseColorFactor.r, baseColorFactor.g, baseColorFactor.b, baseColorFactor.a};

                    materialData["textures"] = nlohmann::json::object();
                    constexpr std::array textureTypes = {
                        aiTextureType_BASE_COLOR, aiTextureType_NORMALS,
                        aiTextureType_EMISSIVE, aiTextureType_METALNESS,
                        aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_LIGHTMAP };

                    for (const auto& textureType : textureTypes) {
                        aiString path;
                        if (m_scene->mMaterials[materialIndex]->GetTexture(textureType, 0, &path) == AI_SUCCESS) {
                            std::string textureTypeName = magic_enum::enum_name(textureType).data();
                            bool textureExists = false;
                            std::string textureUUID;
                            for (const auto& texture : m_metadata["textures"].items()) {
                                if (texture.value()["path"] == m_path + "/" + path.C_Str()) {
                                    textureExists = true;
                                    textureUUID = texture.key();
                                    materialData["textures"][textureTypeName] = texture.key();
                                    break;
                                }
                            }
                            if (textureExists) {
                                continue;
                            }

                            textureUUID = boost::uuids::to_string(generator());
                            auto& textureData = m_metadata["textures"][textureUUID];
                            textureData["path"] = m_path + "/" + path.C_Str();
                            textureData["size"] = m_textureSize;
                        	textureData["normalMap"] = textureType == aiTextureType_NORMALS;
                            materialData["textures"][textureTypeName] = textureUUID;
                        } else {
                        	std::string textureTypeName = magic_enum::enum_name(textureType).data();
                        	std::string textureUUID;
                        	if (textureType == aiTextureType_BASE_COLOR) {
								textureUUID = "00000000-0000-0000-0000-000000000002";
							} else if (textureType == aiTextureType_NORMALS) {
								textureUUID = "00000000-0000-0000-0000-000000000003";
							} else if (textureType == aiTextureType_EMISSIVE) {
								textureUUID = "00000000-0000-0000-0000-000000000002";
							} else if (textureType == aiTextureType_METALNESS) {
								textureUUID = "00000000-0000-0000-0000-000000000001";
							} else if (textureType == aiTextureType_DIFFUSE_ROUGHNESS) {
								textureUUID = "00000000-0000-0000-0000-000000000001";
							} else if (textureType == aiTextureType_LIGHTMAP) {
								textureUUID = "00000000-0000-0000-0000-000000000002";
							}
                        	materialData["textures"][textureTypeName] = textureUUID;
                        }
                    }

                }

                objectData["meshes"].push_back({
                    {"mesh", meshUUID},
                    {"material", materialUUID}
                });
            }


            for (uint32_t i = 0; i < node->mNumChildren; i++) {
                nodes.emplace(node->mChildren[i], objectUUID);
            }
        }

    	for (uint32_t i = 0; i < m_scene->mNumLights; i++) {
    		const auto light = m_scene->mLights[i];
			auto lightUUID = boost::uuids::to_string(generator());
			auto& lightData = m_metadata["lights"][lightUUID];
			lightData["name"] = light->mName.C_Str();
			lightData["type"] = magic_enum::enum_name(light->mType).data();
    		lightData["diffuseColor"] = { light->mColorDiffuse.r, light->mColorDiffuse.g, light->mColorDiffuse.b };
    		lightData["specularColor"] = { light->mColorSpecular.r, light->mColorSpecular.g, light->mColorSpecular.b };
			switch (light->mType) {
				case aiLightSource_DIRECTIONAL: {
					lightData["direction"] = { light->mDirection.x, light->mDirection.y, light->mDirection.z };
					lightData["intensity"] = 1.f;
					break;
				}
				case aiLightSource_POINT: {
					lightData["position"] = { light->mPosition.x, light->mPosition.y, light->mPosition.z };
					lightData["range"] = light->mSize.x;
					lightData["attenuation"] = {
						light->mAttenuationConstant,
						light->mAttenuationLinear,
						light->mAttenuationQuadratic
					};
					break;
				}
				case aiLightSource_SPOT: {
					lightData["position"] = { light->mPosition.x, light->mPosition.y, light->mPosition.z };
					lightData["direction"] = { light->mDirection.x, light->mDirection.y, light->mDirection.z };
					lightData["innerAngle"] = glm::degrees(light->mAngleInnerCone);
					lightData["outerAngle"] = glm::degrees(light->mAngleOuterCone);
					lightData["attenuation"] = {
						light->mAttenuationConstant,
						light->mAttenuationLinear,
						light->mAttenuationQuadratic
					};
					break;
				}
				default:
					break;
			}

    		for (auto& [uuid, objectData] : m_metadata["objects"].items()) {
				if (objectData["name"] == light->mName.C_Str()) {
					if (!objectData.contains("lights")) {
						objectData["lights"] = nlohmann::json::array();
					}
					objectData["lights"].push_back(lightUUID);
					break;
				}
			}
    	}

    	for (uint32_t i = 0; i < m_scene->mNumCameras; i++) {
    		const auto& camera = m_scene->mCameras[i];
			ECS::Camera::Type cameraType = camera->mOrthographicWidth != 0.f ? ECS::Camera::Type::Orthographic : ECS::Camera::Type::Perspective;
    		auto cameraUUID = boost::uuids::to_string(generator());
    		auto& cameraData = m_metadata["cameras"][cameraUUID];
    		cameraData["type"] = magic_enum::enum_name(cameraType).data();
    		if (cameraType == ECS::Camera::Type::Perspective) {
				cameraData["fov"] = glm::degrees(camera->mHorizontalFOV);
				cameraData["near"] = camera->mClipPlaneNear;
				cameraData["far"] = camera->mClipPlaneFar;
			} else {
				cameraData["left"] = -camera->mOrthographicWidth;
				cameraData["right"] = camera->mOrthographicWidth;
				cameraData["bottom"] = -camera->mOrthographicWidth / camera->mAspect;
				cameraData["top"] = camera->mOrthographicWidth / camera->mAspect;
				cameraData["near"] = camera->mClipPlaneNear;
				cameraData["far"] = camera->mClipPlaneFar;
			}

    		std::string cameraName = camera->mName.C_Str();
    		for (auto& [uuid, objectData] : m_metadata["objects"].items()) {
    			if (objectData["name"] == cameraName) {
					if (!objectData.contains("cameras")) {
						objectData["cameras"] = nlohmann::json::array();
					}
					objectData["cameras"].push_back(cameraUUID);
					break;
				}
    		}
    	}

        std::ofstream file("project.json");
        file << m_metadata.dump(4) << std::endl;
        file.close();

    	LoadMeshes();
    	LoadMaterials();
    	Manager::Get().AddPrefab(std::make_unique<Prefab>(m_name, m_metadata));
    }


    void Importer::LoadMeshes() {
        if (m_meshesLoaded)
            return;
        for (const auto& [uuid, meshData] : m_metadata["meshes"].items()) {
            const auto assimpId = meshData["id"].get<uint32_t>();
            const auto mesh = m_scene->mMeshes[assimpId];
			const auto aabb = mesh->mAABB;

            auto builder = Graphics::Mesh::Builder(_stringToUuid(uuid))
                .Name(mesh->mName.C_Str())
        		.AABB(Math::AABB(Math::Vector3<f32>(aabb.mMin), Math::Vector3<f32>(aabb.mMax)));

            for (uint32_t j = 0; j < mesh->mNumVertices; j++) {
                auto position = mesh->mVertices[j];
                auto normal = mesh->mNormals[j];
                auto tangent = mesh->mTangents[j];
                auto bitangent = mesh->mBitangents[j];

                Math::Vector2 texCoord0 = { 0.0f, 0.0f };
                if (mesh->mTextureCoords[0] != nullptr) {
                    texCoord0 = {mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y};
                }
                Math::Vector2 texCoord1 = { 0.0f, 0.0f };
                if (mesh->mTextureCoords[1] != nullptr) {
                    texCoord1 = {mesh->mTextureCoords[1][j].x, mesh->mTextureCoords[1][j].y};
                } else {
	                texCoord1 = texCoord0;
                }
                Color color = { 1.0f, 1.0f, 1.0f, 1.0f };
                if (mesh->mColors[0] != nullptr) {
                    color = {mesh->mColors[0][j].r, mesh->mColors[0][j].g, mesh->mColors[0][j].b, mesh->mColors[0][j].a};
                }

                const auto N = Math::Vector3<f32>(normal).Normalized();
                const auto T = Math::Vector3<f32>(tangent).Normalized();
                const auto B = Math::Vector3<f32>(bitangent).Normalized();
                const auto sign = N.Cross(T).Dot(B) < 0.0f ? -1.0f : 1.0f;

                builder.AddVertex({
                    .position = { position.x, position.y, position.z },
                    .normal = N,
                    .tangent = Math::Vector4(T, sign),
                    .texCoord0 = { texCoord0.x, texCoord0.y },
                    .texCoord1 = { texCoord1.x, texCoord1.y },
                    .color0 = color
                });
            }

            for (uint32_t j = 0; j < mesh->mNumFaces; j++) {
                auto face = mesh->mFaces[j];
                for (uint32_t k = 0; k < face.mNumIndices; k++) {
                    builder.AddIndex(face.mIndices[k]);
                }
            }

            Manager::Get().AddMesh(builder.Build());
        }
        m_meshesLoaded = true;
    }

    void Importer::LoadMaterials() {
        if (m_materialsLoaded)
            return;
        LoadTextures();

  //   	for (const auto&[uuid, texture] : Manager::Get().textures) {
		// 	std::cout << "Loaded texture: " << uuid << " - " << texture->Name() << std::endl;
		// }

        for (const auto& [uuid, materialData] : m_metadata["materials"].items()) {
            auto emissiveFactor = materialData["emissiveFactor"].get<std::array<float, 3>>();
            auto baseColorFactor = materialData["baseColorFactor"].get<std::array<float, 4>>();

            auto builder = Graphics::Material::Builder(_stringToUuid(uuid))
                .Name(materialData["name"].get<std::string>())
                .AlphaCutoff(materialData["alphaCutoff"].get<float>())
                .DoubleSided(materialData["doubleSided"].get<uint32_t>())
                .RoughnessFactor(materialData["roughnessFactor"].get<float>())
                .MetallicFactor(materialData["metallicFactor"].get<float>())
                .EmissiveFactor({ emissiveFactor[0], emissiveFactor[1], emissiveFactor[2] })
                .BaseColorFactor({ baseColorFactor[0], baseColorFactor[1], baseColorFactor[2], baseColorFactor[3] });

            for (const auto& [textureType, textureUUID] : materialData["textures"].items()) {
                const auto textureId = _stringToUuid(textureUUID.get<std::string>());
            	const auto textureEnum = PBR::FromAiTextureType(magic_enum::enum_cast<aiTextureType>(textureType).value());
                builder.AddTexture(textureEnum, Manager::Get().GetTexture(textureId));
            }
            Manager::Get().AddMaterial(builder.Build());
        }

        m_materialsLoaded = true;
    }

    void Importer::LoadTextures() {
        if (m_texturesLoaded)
            return;

    	std::vector<std::pair<std::string, nlohmann::json>> textures;
    	for (const auto& [uuid, textureData] : m_metadata["textures"].items()) {
			textures.emplace_back(uuid, textureData);
		}

    	std::vector<Graphics::Texture::Builder> builders;
    	std::vector<stbi_uc*> datas;
    	std::mutex mtx;
        std::for_each(std::execution::par, textures.begin(), textures.end(), [this, &builders, &datas, &mtx] (const auto& texture) {
        	const auto& [strUuid, textureData] = texture;
            const auto path = textureData["path"].get<std::string>();
            const auto size = textureData["size"].get<uint32_t>();
            const auto textureId = _stringToUuid(strUuid);
            const auto texturePath = m_path + '/' + path;

            int width, height, channels;
            stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!data) {
                throw std::runtime_error("Failed to load texture: " + path);
            }

			{
				std::lock_guard lock(mtx);
				auto& builder = builders.emplace_back(Graphics::Texture::Builder(textureId));

				builder.Name(path.substr(path.find_last_of('/') + 1))
					.Data(data)
					.Width(width)
					.Height(height)
					.Format(vk::Format::eR8G8B8A8Unorm)
					.CreateMipmaps();
	        }
        	datas.emplace_back(data);
        });

    	for (auto& builder : builders) {
    		auto texture = builder.Build();
			if (!texture) {
				throw std::runtime_error("Failed to create texture from builder");
			}
			Manager::Get().AddTexture(std::move(texture));
		}

		for (const auto data : datas) {
			stbi_image_free(data);
    	}

        m_texturesLoaded = true;
    }
}
