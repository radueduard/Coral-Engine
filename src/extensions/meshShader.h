//
// Created by radue on 10/18/2024.
//

#pragma once

#include <vulkan/vulkan.hpp>

namespace Ext {
    class MeshShader {
    public:
        static void ImportFunctions(vk::Instance instance);

        static void cmdDrawMeshTasks(vk::CommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    private:
        static PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT;
    };
}