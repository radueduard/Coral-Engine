//
// Created by radue on 11/6/2024.
//

#include "renderMesh.h"

#include "imgui.h"
#include "graphics/objects/material.h"
#include "graphics/objects/mesh.h"

namespace Coral {
    RenderMesh::RenderMesh(const Object &owner)
        : Component(owner) {}

    void RenderMesh::Add(const Mesh *mesh, const Material *material) {
        m_targets.emplace_back(mesh, material);
    }

    void RenderMesh::Update(double deltaTime) {
    }
}
