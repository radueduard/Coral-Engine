//
// Created by radue on 11/6/2024.
//
#pragma once

#include "graphics/objects/mesh.h"
#include "graphics/objects/material.h"

namespace Coral::ECS {
    class RenderTarget {
    public:
        explicit RenderTarget() = default;
        ~RenderTarget() = default;

        void Add(const Graphics::Mesh *mesh, const Graphics::Material *material);

        [[nodiscard]] const std::vector<std::pair<const Graphics::Mesh*, const Graphics::Material*>>& Targets() const { return m_targets; }

    private:
        std::vector<std::pair<const Graphics::Mesh*, const Graphics::Material*>> m_targets;
    };
}
