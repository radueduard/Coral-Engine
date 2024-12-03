//
// Created by radue on 12/1/2024.
//

#pragma once
#include <glm/glm.hpp>

#include "components/object.h"
#include "components/renderMesh.h"

namespace Graphics {

    class AABB {
    public:
        explicit AABB(const glm::vec3 &min = glm::vec3(0.0f), const glm::vec3 &max = glm::vec3(0.0f), const bool debug = false)
            : m_min(min), m_max(max) {
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

        [[nodiscard]] const glm::vec3 &Min() const { return m_min; }
        [[nodiscard]] const glm::vec3 &Max() const { return m_max; }

        void GrowToInclude(const glm::vec3 &point) {
            m_min = glm::min(m_min, point);
            m_max = glm::max(m_max, point);
        }

        AABB(const AABB &other) = default;
        AABB &operator=(const AABB &other) = default;
    private:
        glm::vec3 m_min {};
        glm::vec3 m_max {};
        std::optional<mgv::Object*> m_debug;
    };

}
