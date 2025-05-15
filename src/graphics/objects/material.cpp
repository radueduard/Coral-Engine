//
// Created by radue on 11/5/2024.
//

#include "material.h"

namespace Coral::Graphics {
    Material::Material(const Builder &builder) : m_uuid(builder.m_uuid), m_name(builder.m_name), m_textures(builder.m_textures) {
        m_parameters = {
            .alphaCutoff = builder.m_alphaCutoff,
            .doubleSided = builder.m_doubleSided,
            .roughnessFactor = builder.m_roughnessFactor,
            .metallicFactor = builder.m_metallicFactor,
            .emissiveFactor = builder.m_emissiveFactor,
            .baseColorFactor = builder.m_baseColorFactor,
        };
    }
}
