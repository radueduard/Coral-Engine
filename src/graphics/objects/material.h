//
// Created by radue on 11/5/2024.
//

#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <boost/uuid/uuid.hpp>
#include <glm/glm.hpp>

#include "texture.h"

namespace Coral {
    struct Parameters {
        float alphaCutoff;
        uint32_t doubleSided;
        float roughnessFactor;
        float metallicFactor;
        alignas(16) glm::vec3 emissiveFactor;
        glm::vec4 baseColorFactor;
    };

    class Material {
    public:
        class Builder {
            friend class Material;
        public:
            Builder(const boost::uuids::uuid& uuid) : m_uuid(uuid) {}

            Builder& Name(const std::string& name) {
                m_name = name;
                return *this;
            }

            Builder& AlphaCutoff(const float alphaCutoff) {
                m_alphaCutoff = alphaCutoff;
                return *this;
            }

            Builder& DoubleSided(const uint32_t doubleSided) {
                m_doubleSided = doubleSided;
                return *this;
            }

            Builder& RoughnessFactor(const float roughnessFactor) {
                m_roughnessFactor = roughnessFactor;
                return *this;
            }

            Builder& MetallicFactor(const float metallicFactor) {
                m_metallicFactor = metallicFactor;
                return *this;
            }

            Builder& EmissiveFactor(const glm::vec3 emissiveFactor) {
                m_emissiveFactor = emissiveFactor;
                return *this;
            }

            Builder& BaseColorFactor(const glm::vec4 baseColorFactor) {
                m_baseColorFactor = baseColorFactor;
                return *this;
            }

            Builder& AddTexture(const PBR::Usage usage, const Coral::Texture* texture) {
                m_textures[usage] = texture;
                return *this;
            }

            std::unique_ptr<Material> Build() {
                return std::make_unique<Material>(*this);
            }
        private:
            boost::uuids::uuid m_uuid;
            std::string m_name;
            float m_alphaCutoff = 0.5f;
            uint32_t m_doubleSided = 0;
            float m_roughnessFactor = 0.5f;
            float m_metallicFactor = 0.5f;
            glm::vec3 m_emissiveFactor = {0.0f, 0.0f, 0.0f};
            glm::vec4 m_baseColorFactor = {1.0f, 1.0f, 1.0f, 1.0f};

            std::unordered_map<PBR::Usage, const Coral::Texture*> m_textures {};
        };

        explicit Material(const Builder& builder);

        [[nodiscard]] const boost::uuids::uuid& UUID() const { return m_uuid; }
        [[nodiscard]] const Coral::Texture* Texture(PBR::Usage usage) const {
            if (m_textures.contains(usage)) {
                return m_textures.at(usage);
            }
            return nullptr;
        }

        [[nodiscard]] const std::string& Name() const { return m_name; }
        [[nodiscard]] const Parameters& Parameters() const { return m_parameters; }

    private:
        boost::uuids::uuid m_uuid;
        std::string m_name;
        Coral::Parameters m_parameters {};
        std::unordered_map<PBR::Usage, const Coral::Texture*> m_textures {};
    };
}
