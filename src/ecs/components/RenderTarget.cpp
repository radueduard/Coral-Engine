//
// Created by radue on 11/6/2024.
//

#include "RenderTarget.h"

#include "graphics/objects/material.h"
#include "graphics/objects/mesh.h"

namespace Coral::ECS {
    void RenderTarget::Add(const Graphics::Mesh *mesh, const Graphics::Material *material) {
        m_targets.emplace_back(mesh, material);
    }
}
