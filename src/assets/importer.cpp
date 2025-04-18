//
// Created by radue on 11/5/2024.
//

#include "importer.h"

#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <stack>
#include <stb_image.h>
#include <boost/uuid/uuid_io.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>

#include "manager.h"
#include "components/renderMesh.h"
#include "graphics/objects/mesh.h"
#include "graphics/objects/textureArray.h"
#include "graphics/objects/material.h"
#include "project/scene.h"

namespace Asset {
    Importer::Importer(const std::string &path) {
        m_path = path.substr(0, path.find_last_of('/'));
        m_name = path.substr(path.find_last_of('/') + 1);

        constexpr auto flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
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
        m_metadata["objects"] = nlohmann::json::object();

        std::stack<std::pair<const aiNode*, std::string>> nodes;
        nodes.emplace(m_scene->mRootNode, boost::uuids::to_string(boost::uuids::nil_uuid()));
        while (!nodes.empty()) {
            const auto [node, parent] = nodes.top();
            nodes.pop();

            aiVector3D position;
            aiQuaternion rotation;
            aiVector3D scale;
            node->mTransformation.Decompose(scale, rotation, position);
            glm::quat rotationQuaternion = {rotation.w, rotation.x, rotation.y, rotation.z};
            glm::vec3 positionVector = {position.x, position.y, position.z};
            glm::vec3 rotationVector = glm::degrees(glm::eulerAngles(rotationQuaternion));
            glm::vec3 scaleVector = {scale.x, scale.y, scale.z};

            auto objectUUID = boost::uuids::to_string(generator());
            auto& objectData = m_metadata["objects"][objectUUID];
            objectData["parent"] = parent;
            objectData["name"] = node->mName.C_Str();
            objectData["transform"] = nlohmann::json::object();
            objectData["transform"]["position"] = {positionVector.x, positionVector.y, positionVector.z};
            objectData["transform"]["rotation"] = {rotationVector.x, rotationVector.y, rotationVector.z};
            objectData["transform"]["scale"] = {scaleVector.x, scaleVector.y, scaleVector.z};
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

                    const auto material = m_scene->mMaterials[i];
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
                    constexpr auto textureTypes = {
                        aiTextureType_BASE_COLOR, aiTextureType_NORMALS,
                        aiTextureType_EMISSIVE, aiTextureType_METALNESS,
                        aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_LIGHTMAP,};

                    for (const auto& textureType : textureTypes) {
                        aiString path;
                        if (m_scene->mMaterials[materialIndex]->GetTexture(textureType, 0, &path) == AI_SUCCESS) {
                            std::string textureTypeName = std::to_string(PBR::Usage(textureType));
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

        std::ofstream file("project.json");
        file << m_metadata.dump(4) << std::endl;
        file.close();
    }


    void Importer::LoadMeshes() const {
        static bool meshesLoaded = false;
        if (meshesLoaded)
            return;
        for (const auto& [uuid, meshData] : m_metadata["meshes"].items()) {
            const auto assimpId = meshData["id"].get<uint32_t>();
            const auto mesh = m_scene->mMeshes[assimpId];
            auto builder = Coral::Mesh::Builder(_stringToUuid(uuid))
                .Name(mesh->mName.C_Str());


            for (uint32_t j = 0; j < mesh->mNumVertices; j++) {
                auto position = mesh->mVertices[j];
                auto normal = mesh->mNormals[j];
                auto tangent = mesh->mTangents[j];
                auto bitangent = mesh->mBitangents[j];

                glm::vec2 texCoord0 = {0.0f, 0.0f};
                if (mesh->mTextureCoords[0] != nullptr) {
                    texCoord0 = {mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y};
                }
                glm::vec2 texCoord1 = {0.0f, 0.0f};
                if (mesh->mTextureCoords[1] != nullptr) {
                    texCoord1 = {mesh->mTextureCoords[1][j].x, mesh->mTextureCoords[1][j].y};
                }
                glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
                if (mesh->mColors[0] != nullptr) {
                    color = {mesh->mColors[0][j].r, mesh->mColors[0][j].g, mesh->mColors[0][j].b, mesh->mColors[0][j].a};
                }

                const auto N = glm::normalize(glm::vec3(normal.x, normal.y, normal.z));
                const auto T = glm::normalize(glm::vec3(tangent.x, tangent.y, tangent.z));
                const auto B = glm::normalize(glm::vec3(bitangent.x, bitangent.y, bitangent.z));
                const auto sign = glm::dot(glm::cross(N, T), B) < 0.0f ? -1.0f : 1.0f;

                builder.AddVertex({
                    .position = {position.x, position.y, position.z},
                    .normal = N,
                    .tangent = {T, sign},
                    .texCoord0 = texCoord0,
                    .texCoord1 = texCoord1,
                    .color0 = color
                });
            }

            for (uint32_t j = 0; j < mesh->mNumFaces; j++) {
                auto face = mesh->mFaces[j];
                for (uint32_t k = 0; k < face.mNumIndices; k++) {
                    builder.AddIndex(face.mIndices[k]);
                }
            }

            Manager::AddMesh(builder.Build());
        }
        meshesLoaded = true;
    }

    void Importer::LoadMaterials() const {

        static bool materialsLoaded = false;
        if (materialsLoaded)
            return;
        LoadTextures();

        for (const auto& [uuid, materialData] : m_metadata["materials"].items()) {
            auto emissiveFactor = materialData["emissiveFactor"].get<std::array<float, 3>>();
            auto baseColorFactor = materialData["baseColorFactor"].get<std::array<float, 4>>();

            auto builder = Coral::Material::Builder(_stringToUuid(uuid))
                .Name(materialData["name"].get<std::string>())
                .AlphaCutoff(materialData["alphaCutoff"].get<float>())
                .DoubleSided(materialData["doubleSided"].get<uint32_t>())
                .RoughnessFactor(materialData["roughnessFactor"].get<float>())
                .MetallicFactor(materialData["metallicFactor"].get<float>())
                .EmissiveFactor({ emissiveFactor[0], emissiveFactor[1], emissiveFactor[2] })
                .BaseColorFactor({ baseColorFactor[0], baseColorFactor[1], baseColorFactor[2], baseColorFactor[3] });

            for (const auto& [textureType, textureUUID] : materialData["textures"].items()) {
                const auto textureId = _stringToUuid(textureUUID.get<std::string>());
                builder.AddTexture(textureType, Manager::GetTexture(textureId));
            }
            Manager::AddMaterial(builder.Build());
        }

        materialsLoaded = true;
    }

    void Importer::LoadTextures() const {
        static bool texturesLoaded = false;
        if (texturesLoaded)
            return;

        for (const auto& [uuid, textureData] : m_metadata["textures"].items()) {
            const auto path = textureData["path"].get<std::string>();
            const auto size = textureData["size"].get<uint32_t>();
            const auto textureId = _stringToUuid(uuid);
            const auto texturePath = m_path + '/' + path;

            int width, height, channels;
            stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!data) {
                throw std::runtime_error("Failed to load texture: " + path);
            }
            auto texture = Coral::Texture::Builder(_stringToUuid(uuid))
                .Name(path)
                .Data(data)
                .Width(width)
                .Height(height)
                .Format(vk::Format::eR8G8B8A8Unorm)
                .Build();
            stbi_image_free(data);
            Manager::AddTexture(std::move(texture));
        }

        texturesLoaded = true;
    }

    GUI::Container<Coral::Scene> Importer::LoadScene() const {
        auto scene = GUI::MakeContainer<Coral::Scene>();

        LoadMeshes();
        LoadMaterials();

        boost::unordered_map<boost::uuids::uuid, Coral::Object*> objectMap;
        for (const auto& [uuid, objectData] : m_metadata["objects"].items()) {
            auto object = new Coral::Object(_stringToUuid(uuid), objectData["name"].get<std::string>());
            object->position = { objectData["transform"]["position"][0].get<float>(),
                                objectData["transform"]["position"][1].get<float>(),
                                objectData["transform"]["position"][2].get<float>() };
            object->rotation = { objectData["transform"]["rotation"][0].get<float>(),
                                objectData["transform"]["rotation"][1].get<float>(),
                                objectData["transform"]["rotation"][2].get<float>() };
            object->scale = { objectData["transform"]["scale"][0].get<float>(),
                            objectData["transform"]["scale"][1].get<float>(),
                            objectData["transform"]["scale"][2].get<float>() };
            objectMap[object->UUID()] = object;
        }

        Coral::Object* root = nullptr;
        for (const auto& [uuid, objectData] : m_metadata["objects"].items()) {
            const auto childUUID = _stringToUuid(uuid);
            const auto parentUUID = _stringToUuid(objectData["parent"].get<std::string>());

            auto* child = objectMap[childUUID];
            if (objectData.contains("meshes")) {
                auto* renderTarget = child->Add<Coral::RenderMesh>();
                for (const auto& meshData : objectData["meshes"]) {
                    const auto meshUUID = _stringToUuid(meshData["mesh"].get<std::string>());
                    const auto materialUUID = _stringToUuid(meshData["material"].get<std::string>());
                    renderTarget->Add(Manager::GetMesh(meshUUID), Manager::GetMaterial(materialUUID));
                }
            }

            if (parentUUID != boost::uuids::nil_uuid()) {
                objectMap[parentUUID]->AddChild(child);
            } else {
                root = objectMap[childUUID];
            }
        }
        scene->Root()->AddChild(root);
        return scene;
    }

}
