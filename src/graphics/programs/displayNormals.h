//
// Created by radue on 11/6/2024.
//

#pragma once

#include "program.h"
#include "components/object.h"
#include "graphics/pipeline.h"
#include "graphics/renderPass.h"
#include "graphics/objects/mesh.h"
#include "memory/buffer.h"


class DisplayNormals : public Graphics::Program {
public:
    struct CreateInfo {
        mgv::Object* object;
        const Memory::Buffer<glm::mat4>& modelBuffer;
        const Memory::Buffer<glm::mat4>& mvpBuffer;
    };

    struct Material {
        float alphaCutoff;
        uint32_t baseColorId;
        uint32_t normalId;
    };

    DisplayNormals(const Core::Device &device, Graphics::RenderPass& renderPass, const Memory::Descriptor::Pool& pool, const CreateInfo& createInfo);
    ~DisplayNormals() override = default;

    void Init() override;
    void Update(double deltaTime) override;
    void Draw(const vk::CommandBuffer& commandBuffer) override;

    void DrawUI() override {}
    void UpdateUI() override {}
private:
    mgv::Object* m_object;
    const Memory::Buffer<glm::mat4>& m_modelBuffer;
    const Memory::Buffer<glm::mat4>& m_mvpBuffer;
};
