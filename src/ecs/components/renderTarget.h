//
// Created by radue on 11/6/2024.
//
#pragma once

#include "component.h"
#include "graphics/objects/material.h"
#include "graphics/objects/mesh.h"

namespace Coral::ECS {
    class RenderTarget final : public Component {
    public:
        explicit RenderTarget() = default;
        ~RenderTarget() override = default;

        void Add(const Graphics::Mesh *mesh, const Graphics::Material *material);

        [[nodiscard]] const std::vector<std::pair<const Graphics::Mesh*, const Graphics::Material*>>& Targets() const { return m_targets; }

    private:
        std::vector<std::pair<const Graphics::Mesh*, const Graphics::Material*>> m_targets;
    };
}
