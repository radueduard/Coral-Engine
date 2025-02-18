//
// Created by radue on 10/18/2024.
//

#include "meshShader.h"

namespace Ext {
    void MeshShader::ImportFunctions(vk::Instance instance) {
        vkCmdDrawMeshTasksEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(
            instance.getProcAddr("vkCmdDrawMeshTasksEXT"));
    }

    void MeshShader::cmdDrawMeshTasks(vk::CommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
        vkCmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }

    PFN_vkCmdDrawMeshTasksEXT MeshShader::vkCmdDrawMeshTasksEXT = nullptr;
}