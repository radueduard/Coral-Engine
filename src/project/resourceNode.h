//
// Created by radue on 11/24/2025.
//

#pragma once

#include <functional>
#include <variant>
#include <vector>

#include "shader/manager.h"

namespace Coral {
	namespace ECS {
		class Entity;
	}

	namespace Memory {
		class Image;
		class Buffer;
	}

	struct SingleResourceNode {
		std::variant<Memory::Buffer*, Memory::Image*> resource;
	};

	struct MultipleResourceNode {
		std::variant<std::vector<Memory::Buffer*>, std::vector<Memory::Image*>> resources;
	};

	struct ResourceNode {
		std::variant<SingleResourceNode*, MultipleResourceNode*> node;
	};

	struct EntityNode {
		ECS::Entity* entity;
	};

	struct GroupNode {
		std::vector<ECS::Entity*> entities;
	};

	struct ResourceGetNode {
		std::function<std::variant<Memory::Buffer*, Memory::Image*>(ECS::Entity*)> getFunction;

		EntityNode* in;
		SingleResourceNode* out;
	};

	struct ResourceGetAllNode {
		std::function<std::variant<std::vector<Memory::Buffer*>, std::vector<Memory::Image*>>(ECS::Entity*)> getAllFunction;

		EntityNode* in;
		MultipleResourceNode* out;
	};


	struct Binding {
		u32 set;
		u32 binding;
	};

	struct PushConstant {
		vk::ShaderStageFlags stage;
		u32 offset;
		u32 size;
	};

	struct ResourceLocator {
		std::variant<Binding, PushConstant> location;
	};

	// TODO ! GENERATE PROGRAM NODE AND ITS INPUTS FROM SHADER REFLECTION, and allow settings for the pipeline

	struct RenderProgramNode {
		std::vector<Shader::Shader*> shaders;

		std::unordered_map<ResourceLocator, ResourceNode*> resources;
		std::vector<ResourceNode*> inputs;
	};

	struct FilterNode {
		std::function<bool(ECS::Entity*)> filterFunction;

		GroupNode* in;
		GroupNode* out;
	};

	struct SelectNode {
		std::function<ECS::Entity*(std::vector<ECS::Entity*>)> selectFunction;

		GroupNode* in;
		EntityNode* out;
	};

	struct TransformNode {
		std::function<ResourceNode(ECS::Entity*)> transformFunction;


	};


}
