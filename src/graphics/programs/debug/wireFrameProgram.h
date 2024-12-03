//
// Created by radue on 12/1/2024.
//

#pragma once

#include "../program.h"
#include "components/object.h"

class WireFrameProgram final : public Graphics::Program {
public:
    WireFrameProgram(Graphics::RenderPass &renderPass, const Memory::Descriptor::Pool &pool);
    ~WireFrameProgram() override = default;

    void InitUI() override {}
    void UpdateUI() override {}
    void DrawUI() override {}
    void DestroyUI() override {}

    void Init() override {}
    void Update(double deltaTime) override {}
    void Draw(const vk::CommandBuffer &commandBuffer, bool reflected) override;

    void Add(mgv::Object* object) { m_objects.emplace_back(object); }
    void Remove(mgv::Object* object) { std::erase(m_objects, object); }
private:
    std::vector<mgv::Object*> m_objects;
};
