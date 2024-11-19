//
// Created by radue on 11/5/2024.
//

#include "material.h"

namespace mgv {
    Material::Material(const std::string& name, const Builder &builder) {
        m_name = name;
        m_parameters = {
            .alphaCutoff = builder.m_alphaCutoff,
            .doubleSided = builder.m_doubleSided,
            .roughnessFactor = builder.m_roughnessFactor,
            .metallicFactor = builder.m_metallicFactor,
            .emissiveFactor = builder.m_emissiveFactor,
            .baseColorFactor = builder.m_baseColorFactor,
            .baseColorId = builder.m_baseColorId,
            .normalId = builder.m_normalId,
        };
    }
}
