//
// Created by radue on 11/5/2024.
//

#pragma once
#include <memory>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <glm/glm.hpp>

namespace mgv {
    struct Parameters {
        float alphaCutoff;
        uint32_t doubleSided;
        float roughnessFactor;
        float metallicFactor;
        alignas(16) glm::vec3 emissiveFactor;
        glm::vec4 baseColorFactor;

        uint32_t baseColorId;
        uint32_t normalId;
    };

    class Material {
    public:
        class Builder {
            friend class Material;
        public:
            Builder() = default;

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

            Builder& BaseColorId(const uint32_t baseColorId) {
                m_baseColorId = baseColorId;
                return *this;
            }

            Builder& NormalId(const uint32_t normalId) {
                m_normalId = normalId;
                return *this;
            }

            std::unique_ptr<Material> Build(std::string name) {
                return std::make_unique<Material>(name, *this);
            }
        private:
            float m_alphaCutoff;
            uint32_t m_doubleSided;
            float m_roughnessFactor;
            float m_metallicFactor;
            glm::vec3 m_emissiveFactor;
            glm::vec4 m_baseColorFactor;

            uint32_t m_baseColorId;
            uint32_t m_normalId;
        };

    explicit Material(const std::string& name, const Builder& builder);

    [[nodiscard]] const std::string& Name() const { return m_name; }
    [[nodiscard]] const Parameters& Parameters() const { return m_parameters; }

    private:
        std::string m_name;
        mgv::Parameters m_parameters {};
    };
}