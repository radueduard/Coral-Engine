//
// Created by radue on 11/6/2024.
//

#pragma once
#include "object.h"
#include "graphics/objects/mesh.h"

namespace mgv {
    class Material;
}

namespace mgv {
    class RenderMesh final : public Component {
    public:
        explicit RenderMesh(const Object &owner);
        ~RenderMesh() override = default;

        void Add(const Mesh *mesh, const Material *material);

        void Update(double deltaTime) override;

        [[nodiscard]] const std::vector<std::pair<const Mesh*, const Material*>>& Targets() const { return m_targets; }

    private:
        std::vector<std::pair<const Mesh*, const Material*>> m_targets;
    };
}
