//
// Created by radue on 12/1/2024.
//

#pragma once
#include <optional>
#include <glm/glm.hpp>

namespace mgv {
    class Object;
}

namespace Graphics {
    class AABB {
    public:
        explicit AABB(const glm::vec3 &min = glm::vec3(0.0f), const glm::vec3 &max = glm::vec3(0.0f), const bool debug = false);

        [[nodiscard]] const glm::vec3 &Min() const { return m_min; }
        [[nodiscard]] const glm::vec3 &Max() const { return m_max; }

        void GrowToInclude(const glm::vec3 &point) {
            m_min = min(m_min, point);
            m_max = max(m_max, point);
        }

        AABB(const AABB &other) = default;
        AABB &operator=(const AABB &other) = default;
    private:
        glm::vec3 m_min {};
        glm::vec3 m_max {};
        std::optional<mgv::Object*> m_debug;
    };

}
