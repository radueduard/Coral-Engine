//
// Created by radue on 11/5/2024.
//

#include "importer.h"

#include <queue>
#include <set>
#include <stb_image.h>
#include <boost/uuid/uuid_io.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>

#include "manager.h"
#include "components/renderMesh.h"
#include "graphics/objects/mesh.h"
#include "graphics/objects/textureArray.h"

namespace Asset {
    boost::uuids::string_generator Importer::_stringToUuid;
    Assimp::Importer Importer::_importer;

    Importer::Importer(Core::Device &device, const std::string &path) : m_device(device), m_metadata(path)
    {
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

    void Importer::LoadMeshes() {
        for (uint32_t i = 0; i < m_scene->mNumMeshes; i++) {
            const auto mesh = m_scene->mMeshes[i];
            auto builder = mgv::Mesh::Builder(mesh->mName.C_Str());

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

            auto meshId = Manager::AddMesh(builder.Build(m_device));
            m_metadata.AddMeshMapping(to_string(meshId), i);
        }
    }

    void Importer::LoadChildren(const aiNode* parentNode, const std::unique_ptr<mgv::Object>& parentObject) const {
        for (uint32_t i = 0; i < parentNode->mNumChildren; i++) {
            const auto childNode = parentNode->mChildren[i];
            auto childObject = std::make_unique<mgv::Object>(childNode->mName.C_Str());

            aiVector3D position;
            aiQuaternion rotation;
            aiVector3D scale;
            childNode->mTransformation.Decompose(scale, rotation, position);

            glm::quat rotationQuaternion = {rotation.w, rotation.x, rotation.y, rotation.z};

            childObject->position = {position.x, position.y, position.z};
            childObject->rotation = glm::degrees(glm::eulerAngles(rotationQuaternion));
            childObject->scale = {scale.x, scale.y, scale.z};

            if (childNode->mNumMeshes > 0) {
                childObject->Add<mgv::RenderMesh>();
                auto* renderMesh = childObject->Get<mgv::RenderMesh>().value();
                for (uint32_t j = 0; j < childNode->mNumMeshes; j++) {
                    const auto meshId = childNode->mMeshes[j];
                    const auto materialIndex = m_scene->mMeshes[meshId]->mMaterialIndex;

                    const auto meshUUID = _stringToUuid(m_metadata.GetMeshUUID(meshId));
                    const auto materialUUID = _stringToUuid(m_metadata.GetMaterialUUID(materialIndex));

                    const auto mesh = Manager::GetMesh(meshUUID);
                    const auto material = Manager::GetMaterial(materialUUID);

                    if (mesh != nullptr && material != nullptr) {
                        renderMesh->Add(mesh, material);
                    }
                }
            }
            LoadChildren(childNode, childObject);
            parentObject->AddChild(std::move(childObject));
        }
    }

    void Importer::LoadMaterials() {
        for (uint32_t i = 0; i < m_scene->mNumMaterials; i++) {
            const auto material = m_scene->mMaterials[i];
            aiString name;
            material->Get(AI_MATKEY_NAME, name);

            ai_real alphaCutoff;
            material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff);

            ai_int doubleSided;
            material->Get(AI_MATKEY_TWOSIDED, doubleSided);

            ai_real roughnessFactor;
            material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor);

            ai_real metallicFactor;
            material->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor);

            aiColor3D emissiveFactor;
            material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveFactor);

            aiColor4D baseColorFactor;
            material->Get(AI_MATKEY_BASE_COLOR, baseColorFactor);

            aiString baseColorTexturePath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &baseColorTexturePath);

            aiString normalTexturePath;
            material->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath);

            auto mat = mgv::Material::Builder()
                .AlphaCutoff(alphaCutoff)
                .DoubleSided(doubleSided)
                .RoughnessFactor(roughnessFactor)
                .MetallicFactor(metallicFactor)
                .EmissiveFactor({emissiveFactor.r, emissiveFactor.g, emissiveFactor.b})
                .BaseColorFactor({baseColorFactor.r, baseColorFactor.g, baseColorFactor.b, baseColorFactor.a})
                .BaseColorId(Asset::Manager::TextureIdInArray("baseColor", m_path + '/' + baseColorTexturePath.C_Str()))
                .NormalId(Asset::Manager::TextureIdInArray("normal", m_path + '/' + normalTexturePath.C_Str()))
                .Build(name.C_Str());

            const auto matId = Manager::AddMaterial(std::move(mat));
            m_metadata.AddMaterialMapping(to_string(matId), i);
        }
    }

    void Importer::LoadBaseColorTextures() const {
        std::set<std::string> textures;
        for (uint32_t i = 0; i < m_scene->mNumMaterials; i++) {
            aiString texturePath;
            if (m_scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
                textures.insert(texturePath.C_Str());
            }
        }

        auto builder = mgv::TextureArray::Builder()
            .Name("baseColor")
            .ImageSize(m_textureSize)
            .Format(vk::Format::eR8G8B8A8Srgb);

        for (const auto& texture : textures) {
            builder.AddImagePath(m_path + "/" + texture);
        }

        auto textureArray =  builder.Build();
        Asset::Manager::AddTextureArray("baseColor", std::move(textureArray));
    }

    void Importer::LoadNormalTextures() const {
        std::set<std::string> textures;
        for (uint32_t i = 0; i < m_scene->mNumMaterials; i++) {
            aiString texturePath;
            if (m_scene->mMaterials[i]->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
                textures.insert(texturePath.C_Str());
            }
        }

        auto builder = mgv::TextureArray::Builder()
            .Name("normal")
            .ImageSize(m_textureSize)
            .Format(vk::Format::eR8G8B8A8Unorm);

        for (const auto& texture : textures) {
            builder.AddImagePath(m_path + "/" + texture);
        }

        auto textureArray =  builder.Build();
        Asset::Manager::AddTextureArray("normal", std::move(textureArray));
    }

    void Importer::LoadMetallicRoughnessTextures() const {
        std::set<std::string> textures;
        for (uint32_t i = 0; i < m_scene->mNumMaterials; i++) {
            aiString texturePath;
            if (m_scene->mMaterials[i]->GetTexture(aiTextureType_METALNESS, 0, &texturePath) == AI_SUCCESS) {
                textures.insert(texturePath.C_Str());
            }
            if (m_scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texturePath) == AI_SUCCESS) {
                textures.insert(texturePath.C_Str());
            }
        }

        auto builder = mgv::TextureArray::Builder()
            .Name("metallicRoughness")
            .ImageSize(m_textureSize)
            .Format(vk::Format::eR8G8B8A8Srgb);

        for (const auto& texture : textures) {
            builder.AddImagePath(m_path + "/" + texture);
        }

        auto textureArray = builder.Build();
        Asset::Manager::AddTextureArray("metallicRoughness", std::move(textureArray));
    }

    void Importer::LoadOcclusionTextures() const {
        std::set<std::string> textures;
        for (uint32_t i = 0; i < m_scene->mNumMaterials; i++) {
            for (uint32_t j = 0; j < m_scene->mMaterials[i]->mNumProperties; j++) {
                const auto prop = m_scene->mMaterials[i]->mProperties[j];
                std::cout << prop->mKey.C_Str() << std::endl;
            }

            aiString texturePath;
            if (m_scene->mMaterials[i]->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texturePath) == AI_SUCCESS) {
                textures.insert(texturePath.C_Str());
            }
        }

        auto builder = mgv::TextureArray::Builder()
            .Name("occlusion")
            .ImageSize(m_textureSize)
            .Format(vk::Format::eR8G8B8A8Srgb);

        for (const auto& texture : textures) {
            builder.AddImagePath(m_path + "/" + texture);
        }

        auto textureArray =  builder.Build();
        Asset::Manager::AddTextureArray("occlusion", std::move(textureArray));
    }

    void Importer::LoadEmissiveTextures() const {
        std::set<std::string> textures;
        for (uint32_t i = 0; i < m_scene->mNumMaterials; i++) {
            aiString texturePath;
            if (m_scene->mMaterials[i]->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) == AI_SUCCESS) {
                textures.insert(texturePath.C_Str());
            }
        }

        auto builder = mgv::TextureArray::Builder()
            .Name("emissive")
            .ImageSize(m_textureSize)
            .Format(vk::Format::eR8G8B8A8Srgb);

        for (const auto& texture : textures) {
            builder.AddImagePath(m_path + "/" + texture);
        }

        auto textureArray =  builder.Build();
        Asset::Manager::AddTextureArray("emissive", std::move(textureArray));
    }

    void Importer::LoadTextures() const {
        LoadBaseColorTextures();
        LoadNormalTextures();
        LoadMetallicRoughnessTextures();
        // LoadOcclusionTextures();
        // LoadEmissiveTextures();
    }

    std::unique_ptr<mgv::Scene> Importer::LoadScene() const {
        auto scene = std::make_unique<mgv::Scene>();

        const auto rootNode = m_scene->mRootNode;
        auto& root = scene->Root();

        aiVector3D position;
        aiQuaternion rotation;
        aiVector3D scale;
        rootNode->mTransformation.Decompose(scale, rotation, position);

        const glm::quat rotationQuaternion = {rotation.w, rotation.x, rotation.y, rotation.z};

        root->position = {position.x, position.y, position.z};
        root->rotation = glm::eulerAngles(rotationQuaternion);
        root->scale = {scale.x, scale.y, scale.z};

        if (rootNode->mNumMeshes > 0) {
            root->Add<mgv::RenderMesh>();
            mgv::RenderMesh* renderMesh = root->Get<mgv::RenderMesh>().value();
            for (uint32_t j = 0; j < rootNode->mNumMeshes; j++) {
                const auto meshId = rootNode->mMeshes[j];
                const auto aiMesh = m_scene->mMeshes[meshId];

                const auto meshUUID = _stringToUuid(m_metadata.GetMeshUUID(meshId));
                const auto materialUUID = _stringToUuid(m_metadata.GetMaterialUUID(aiMesh->mMaterialIndex));

                const auto mesh = Manager::GetMesh(meshUUID);
                const auto material = Manager::GetMaterial(materialUUID);
                if (mesh != nullptr && material != nullptr) {
                    renderMesh->Add(mesh, material);
                }
            }
        }
        LoadChildren(rootNode, root);
        return scene;
    }

}
