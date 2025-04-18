//
// Created by radue on 10/17/2024.
//

#pragma once

#include <filesystem>
#include <set>
#include <string>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <vulkan/vulkan.hpp>

#include "memory/descriptor/setLayout.h"

namespace Core {
    class Stage {
    public:
        enum class Values : uint32_t
        {
            Vertex                  = 1 << 0,
            TessellationControl     = 1 << 1,
            TessellationEvaluation  = 1 << 2,
            Geometry                = 1 << 3,
            Fragment                = 1 << 4,
            AllGraphics             = Vertex | TessellationControl | TessellationEvaluation | Geometry | Fragment,
            Compute                 = 1 << 5,
            Task                    = 1 << 6,
            Mesh                    = 1 << 7,
            Raygen                  = 1 << 8,
            AnyHit                  = 1 << 9,
            ClosestHit              = 1 << 10,
            Miss                    = 1 << 11,
            Intersection            = 1 << 12,
            AllRayTracing           = Raygen | AnyHit | ClosestHit | Miss | Intersection,
            Callable                = 1 << 13,
            All                     = 0x7FFFFFFF
        };

        Stage(const Values value) : m_value(value) {}
        Stage(const vk::ShaderStageFlagBits value) : m_value(static_cast<Values>(value)) {}

        [[nodiscard]] bool operator==(const Stage &other) const { return m_value == other.m_value; }
        [[nodiscard]] bool operator==(const Values &other) const { return m_value == other; }

        auto operator<=>(const Stage &other) const {
            return m_value <=> other.m_value;
        }

        uint32_t operator &(const Stage &other) const { return static_cast<uint32_t>(m_value) & static_cast<uint32_t>(other.m_value); }

        [[nodiscard]] uint32_t Value() const { return static_cast<uint32_t>(m_value); }

        static constexpr std::vector<Values> AllValues() {
            return {
                Values::Vertex,
                Values::TessellationControl,
                Values::TessellationEvaluation,
                Values::Geometry,
                Values::Fragment,
                Values::Compute,
                Values::Task,
                Values::Mesh,
                Values::Raygen,
                Values::AnyHit,
                Values::ClosestHit,
                Values::Miss,
                Values::Intersection
            };
        }

        [[nodiscard]] bool IsGraphics() const { return *this & Values::AllGraphics; }
        [[nodiscard]] bool IsCompute() const { return *this & Values::Compute; }
        [[nodiscard]] bool IsRayTracing() const { return *this & Values::AllRayTracing; }

        operator Values() const {
            return m_value;
        }

        operator vk::ShaderStageFlagBits() const {
            return static_cast<vk::ShaderStageFlagBits>(static_cast<uint32_t>(m_value));
        }

        operator vk::ShaderStageFlags() const {
            return static_cast<vk::ShaderStageFlagBits>(m_value);
        }

    private:
        Values m_value;
    };

    class Shader final : public EngineWrapper<vk::ShaderModule> {
    public:
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
        [[nodiscard]] Core::Stage Stage() const { return m_stage; }

        [[nodiscard]] const std::set<Descriptor> &Descriptors() const { return m_descriptors; }
        [[nodiscard]] const std::vector<PushConstantRange> &PushConstantRanges() const { return m_pushConstantRanges; }
        [[nodiscard]] const std::vector<uint32_t> &SpirVCode() const { return m_spirVCode; }
        [[nodiscard]] const nlohmann::json& Analysis() const { return m_analysis; }

        nlohmann::json AnalyzeShader() const;

    private:
        std::filesystem::path m_path;
        Core::Stage m_stage;

        std::vector<uint32_t> m_spirVCode;

        std::set<Descriptor> m_descriptors {};
        std::vector<PushConstantRange> m_pushConstantRanges {};

        nlohmann::json m_analysis;

        [[nodiscard]] static vk::ShaderModule LoadSpirVShader(const std::vector<uint32_t> &);
        static std::vector<uint32_t> CompileGLSLToSpirV(const std::string &, const vk::ShaderStageFlagBits &);
        void LoadResourceInfo();
    };
}

namespace std {
    template<>
    struct hash<Core::Stage> {
        size_t operator()(const Core::Stage &stage) const noexcept {
            return hash<uint32_t>()(stage.Value());
        }
    };

    inline string to_string(const Core::Stage::Values &stage)
    {
        switch (stage) {
            case Core::Stage::Values::Vertex: return "Vertex";
            case Core::Stage::Values::TessellationControl: return "TessellationControl";
            case Core::Stage::Values::TessellationEvaluation: return "TessellationEvaluation";
            case Core::Stage::Values::Geometry: return "Geometry";
            case Core::Stage::Values::Fragment: return "Fragment";
            case Core::Stage::Values::Compute: return "Compute";
            case Core::Stage::Values::Task: return "Task";
            case Core::Stage::Values::Mesh: return "Mesh";
            case Core::Stage::Values::Raygen: return "Raygen";
            case Core::Stage::Values::AnyHit: return "AnyHit";
            case Core::Stage::Values::ClosestHit: return "ClosestHit";
            case Core::Stage::Values::Miss: return "Miss";
            case Core::Stage::Values::Intersection: return "Intersection";
            default: throw std::runtime_error("Unknown shader stage");
        }
    }
}
