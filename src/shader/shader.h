//
// Created by radue on 11/22/2025.
//

#pragma once

#include <set>

#include "utils/globalWrapper.h"
#include "utils/types.h"

#include <vulkan/vulkan.hpp>

namespace Coral::Shader {
	enum class Stage : u32 {
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

	struct InOut {
		uint32_t location;
		std::string name;
		vk::Format format;
		std::string semantic;

		auto operator<=>(const InOut& other) const {
			return location <=> other.location;
		}
	};

	struct Descriptor {
		uint32_t set;
		uint32_t binding;
		std::string name;
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
		std::string name;
	};

	class Shader : public EngineWrapper<vk::ShaderModule> {
	public:
		~Shader() override;

		[[nodiscard]] Stage GetStage() const { return m_stage; }
		[[nodiscard]] const std::set<InOut>& Inputs() const { return m_inputs; }
		[[nodiscard]] const std::set<InOut>& Outputs() const { return m_outputs; }
		[[nodiscard]] const std::set<Descriptor>& Descriptors() const { return m_descriptors; }
		[[nodiscard]] const std::vector<PushConstantRange>& PushConstantRanges() const { return m_pushConstantRanges; }

		void PrintLayoutInfo() const;

		virtual void Update() = 0;
		bool HasReloaded() const { return m_reloaded; }
		void LateUpdate() { m_reloaded = false; }

	protected:
		Stage m_stage = Stage::All;
		std::vector<uint32_t> m_spirVCode;
		bool m_valid = false;

		bool m_reloaded = false;

		std::set<InOut> m_inputs {};
		std::set<InOut> m_outputs {};
		std::set<Descriptor> m_descriptors {};
		std::vector<PushConstantRange> m_pushConstantRanges {};


		void LoadSpirVShader();
		void LoadResourceInfo(std::unordered_map<std::string, std::string> semanticMap = {});
	};

	class SlangShader final : public Shader {
	public:
		SlangShader(const std::string& module, const std::string& entryPoint);

		void Update() override;

	private:
		std::filesystem::path m_path;
		std::filesystem::file_time_type m_lastWriteTime;

		std::string m_module;
		std::string m_entryPoint;
		bool m_changed = false;
		std::unordered_map<std::string, std::string> m_semanticMap;

		void Compile();
	};
}