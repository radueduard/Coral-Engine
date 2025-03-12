//
// Created by radue on 10/17/2024.
//

#pragma once

#include <filesystem>
#include <set>
#include <string>
#include <unordered_map>

#include <vulkan/vulkan.hpp>

#include "memory/descriptor/setLayout.h"

namespace Core {
    class Shader final : public EngineWrapper<vk::ShaderModule> {
    public:
        enum Type : uint8_t {
            Vertex = 0,
            TessellationControl = 1,
            TessellationEvaluation = 2,
            Geometry = 3,
            Fragment = 4,
            Task = 5,
            Mesh = 6,
            Compute = 7,
            Count = 8
        };

        struct Descriptor {
            uint32_t set;
            uint32_t binding;
            vk::DescriptorType type;
            uint32_t count;

            auto operator<=>(const Descriptor & other) const {
                if (set != other.set) {
                    return set <=> other.set;
                }
                return binding <=> other.binding;
            }
        };

        struct PushConstantRange {
            uint32_t size;
            uint32_t offset;
        };

        explicit Shader(const std::filesystem::path &, const vk::ShaderStageFlagBits & = vk::ShaderStageFlagBits::eAllGraphics);
        ~Shader() override;

        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;

        [[nodiscard]] std::string Name() const { return m_path.filename().string(); }
        [[nodiscard]] const std::filesystem::path& Path() const { return m_path; }
        [[nodiscard]] const vk::ShaderStageFlagBits &Stage() const { return m_stage; }

        [[nodiscard]] const std::set<Descriptor> &Descriptors() const { return m_descriptors; }
        [[nodiscard]] const std::vector<PushConstantRange> &PushConstantRanges() const { return m_pushConstantRanges; }


    private:
        std::filesystem::path m_path;
        vk::ShaderStageFlagBits m_stage;

        std::vector<uint32_t> m_spirVCode;

        std::set<Descriptor> m_descriptors {};
        std::vector<PushConstantRange> m_pushConstantRanges {};

        [[nodiscard]] static vk::ShaderModule LoadSpirVShader(const std::vector<uint32_t> &);
        static std::vector<uint32_t> CompileGLSLToSpirV(const std::string &, const vk::ShaderStageFlagBits &);
        void LoadResourceInfo();
    };

}
