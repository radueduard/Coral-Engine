//
// Created by radue on 12/6/2024.
//

#include "manager.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "buffer.h"
#include "imgui.h"
#include "core/physicalDevice.h"

void Memory::Manager::DrawUI() {
    ImGui::Begin("Memory Manager");
    static bool createBuffer = false;
    if (ImGui::Button("Create Buffer")) {
        createBuffer = true;
    }

    if (createBuffer) {
        ImGui::Begin("Create Buffer", &createBuffer);
        static vk::DeviceSize instanceSize = 0;
        static uint32_t instanceCount = 0;
        static vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
        static vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        static vk::DeviceSize minOffsetAlignment = 0;

        ImGui::InputScalar("Instance size", ImGuiDataType_U64, &instanceSize);
        ImGui::InputScalar("Instance count", ImGuiDataType_U32, &instanceCount);
        ImGui::InputScalar("Usage flags", ImGuiDataType_U32, &usageFlags);

        if (ImGui::Button("Create")) {
            m_instance->m_buffers[boost::uuids::random_generator()()] =
                std::make_unique<Buffer>(instanceSize, instanceCount,
                    usageFlags, memoryPropertyFlags, minOffsetAlignment);
            createBuffer = false;
        }
        ImGui::End();
    }

    if (ImGui::BeginListBox("Buffers")) {
        for (const auto &[uuid, buffer] : m_instance->m_buffers) {
            static bool selected = false;
            if (selected = ImGui::Selectable(to_string(uuid).c_str(), selected); selected) {
                ImGui::BeginChild("Buffer Details");
                ImGui::Text("UUID: %s", to_string(uuid).c_str());
                ImGui::Text("Size: %lu", buffer->Size());
                ImGui::Text("Instance size: %lu", buffer->InstanceSize());
                ImGui::Text("Instance count: %u", buffer->InstanceCount());
                ImGui::Text("Alignment size: %lu", buffer->AlignmentSize());
                ImGui::Text("Usage flags: %u", buffer->UsageFlags());
                ImGui::Text("Memory property flags: %u", buffer->MemoryPropertyFlags());
                ImGui::EndChild();
            }
        }
        ImGui::EndListBox();
    }
    ImGui::End();
}
