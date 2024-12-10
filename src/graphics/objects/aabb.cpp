//
// Created by radue on 12/1/2024.
//

#include "aabb.h"

#include "mesh.h"
#include "components/object.h"
#include "components/renderMesh.h"

namespace Graphics {
    AABB::AABB(const glm::vec3 &min, const glm::vec3 &max, const bool debug): m_min(min), m_max(max) {
        if (debug) {
            m_debug = new mgv::Object("AABB");
            m_debug.value()->scale = glm::vec3(m_max - m_min);
            m_debug.value()->position = m_min + (m_max - m_min) / 2.0f;
            const auto renderMesh = std::make_shared<mgv::RenderMesh>(*m_debug.value());
            const auto* mesh = mgv::Mesh::Cube();
            renderMesh->Add(mesh, nullptr);
        } else {
            m_debug = std::nullopt;
        }
    }
} // Graphics