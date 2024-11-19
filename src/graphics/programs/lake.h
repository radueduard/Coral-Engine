//
// Created by radue on 11/17/2024.
//

#pragma once


#include "components/camera.h"
#include "graphics/objects/texture.h"
#include "graphics/objects/textureArray.h"
#include "graphics/programs/program.h"

class Lake final : public Graphics::Program {
public:
    struct CreateInfo {
        const mgv::Camera &camera;
    };

    explicit Lake(Core::Device &device, Graphics::RenderPass &renderPass, const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo);
    ~Lake() override = default;

    void Init() override;
    void Update(double deltaTime) override;
    void Draw(const vk::CommandBuffer &commandBuffer) override;

    void InitUI() override;
    void UpdateUI() override;
    void DrawUI() override;
    void DestroyUI() override;

private:
    Core::Device &m_device;
    const mgv::Camera &m_camera;

    glm::ivec2 patchCount = { 100, 100 };
};

