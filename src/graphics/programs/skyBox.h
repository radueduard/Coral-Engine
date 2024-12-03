//
// Created by radue on 11/29/2024.
//

#pragma once
#include "program.h"
#include "components/camera.h"
#include "graphics/objects/cubeMap.h"

class SkyBox final : public Graphics::Program {
public:
    struct CreateInfo {
        const mgv::Camera &camera = *mgv::Camera::mainCamera;
        const Graphics::CubeMap &cubeMap;
    };

    explicit SkyBox(Graphics::RenderPass &renderPass, const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo);
    ~SkyBox() override = default;

    void Init() override {}
    void Update(double deltaTime) override {}
    void Draw(const vk::CommandBuffer &commandBuffer, bool reflected) override;

    void InitUI() override {}
    void UpdateUI() override {}
    void DrawUI() override {}
    void DestroyUI() override {}
private:
    const mgv::Camera &m_camera;
};
