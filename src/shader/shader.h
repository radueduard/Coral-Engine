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

namespace Coral::Shader {
	class Manager;
}
namespace Coral::Core {
    enum class Stage : uint32_t {
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

	inline Stage operator|(Stage lhs, Stage rhs) {
		return static_cast<Stage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}

	inline Stage& operator|=(Stage &lhs, Stage rhs) {
		lhs = static_cast<Stage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
		return lhs;
	}

	inline Stage operator&(Stage lhs, Stage rhs) {
		return static_cast<Stage>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
	}

	inline Stage& operator&=(Stage &lhs, Stage rhs) {
		lhs = static_cast<Stage>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
		return lhs;
	}

    class Shader final : public EngineWrapper<vk::ShaderModule> {
		friend class Coral::Shader::Manager;
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
        [[nodiscard]] Stage Stage() const { return m_stage; }

        [[nodiscard]] const std::set<Descriptor> &Descriptors() const { return m_descriptors; }
        [[nodiscard]] const std::vector<PushConstantRange> &PushConstantRanges() const { return m_pushConstantRanges; }
        [[nodiscard]] const std::vector<uint32_t> &SpirVCode() const { return m_spirVCode; }
        [[nodiscard]] const nlohmann::json& Analysis() const { return m_analysis; }

		[[nodiscard]] std::filesystem::file_time_type LastWriteTime() const { return m_lastWriteTime; }

        nlohmann::json AnalyzeShader() const;

		void Reload();

		[[nodiscard]] bool Changed() const { return m_changed; }
		[[nodiscard]] bool Valid() const { return m_valid; }

    private:
        std::filesystem::path m_path;
		std::filesystem::file_time_type m_lastWriteTime;
        Core::Stage m_stage;

		bool m_valid = true;
		bool m_changed = false;

        std::vector<uint32_t> m_spirVCode;

        std::set<Descriptor> m_descriptors {};
        std::vector<PushConstantRange> m_pushConstantRanges {};

        nlohmann::json m_analysis;

        [[nodiscard]] static vk::ShaderModule LoadSpirVShader(const std::vector<uint32_t> &);
        static std::vector<uint32_t> CompileGLSLToSpirV(const std::string &, const vk::ShaderStageFlagBits &);
        void LoadResourceInfo();
    };
}

