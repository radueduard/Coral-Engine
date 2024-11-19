//
// Created by radue on 11/6/2024.
//

#include "renderMesh.h"

#include "imgui.h"

namespace mgv {
    RenderMesh::RenderMesh(const Object &owner)
        : Component(owner) {}

    void RenderMesh::Add(const Mesh *mesh, const Material *material) {
        m_targets.emplace_back(mesh, material);
    }

    void RenderMesh::DrawUI() {
        ImGui::BeginListBox("Meshes");
        for (const auto &[mesh, material]: m_targets) {
            ImGui::Text("%s - %s", mesh->Name().c_str(), material->Name().c_str());
        }
        ImGui::EndListBox();
    }

    void RenderMesh::Update(double deltaTime) {
    }
}
