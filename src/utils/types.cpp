//
// Created by radue on 11/23/2025.
//

#include "types.h"

#include <spirv_cross.hpp>
#include <vulkan/vulkan.hpp>

vk::Format Coral::FormatFromString(const String& format) {
	if (format == "float" || format == "bool" || format == "int" || format == "uint") {
		return vk::Format::eR32Sfloat;
	}
	if (format == "vec2" || format == "bvec2" || format == "ivec2 " || format == "uvec2") {
		return vk::Format::eR32G32Sfloat;
	}
	if (format == "vec3" || format == "bvec3" || format == "ivec3" || format == "uvec3") {
		return vk::Format::eR32G32B32Sfloat;
	}
	if (format == "vec4" || format == "bvec4" || format == "ivec4" || format == "uvec4") {
		return vk::Format::eR32G32B32A32Sfloat;
	}
	if (format == "mat2" || format == "dmat2") {
		return vk::Format::eR32G32Sfloat;
	}
	if (format == "mat3" || format == "dmat3") {
		return vk::Format::eR32G32B32Sfloat;
	}
	if (format == "mat4" || format == "dmat4") {
		return vk::Format::eR32G32B32A32Sfloat;
	}
	throw std::runtime_error("Unknown format");
}

vk::Format Coral::SPIRTypeToVkFormatConverter(const spirv_cross::SPIRType& type) {
	auto rows = type.vecsize;
	auto columns = type.columns;
	auto base = type.basetype;

	if (columns > 1) {
		// matrices are not supported
		return vk::Format::eUndefined;
	}

	switch (base) {
		case spirv_cross::SPIRType::BaseType::Float: {
			switch (rows) {
				case 1: return vk::Format::eR32Sfloat;
				case 2: return vk::Format::eR32G32Sfloat;
				case 3: return vk::Format::eR32G32B32Sfloat;
				case 4: return vk::Format::eR32G32B32A32Sfloat;
				default: return vk::Format::eUndefined;
			}
		}
		case spirv_cross::SPIRType::BaseType::Double: {
			switch (rows) {
				case 1: return vk::Format::eR64Sfloat;
				case 2: return vk::Format::eR64G64Sfloat;
				case 3: return vk::Format::eR64G64B64Sfloat;
				case 4: return vk::Format::eR64G64B64A64Sfloat;
				default: return vk::Format::eUndefined;
			}
		}
		case spirv_cross::SPIRType::BaseType::SByte: {
			switch (rows) {
				case 1: return vk::Format::eR8Sint;
				case 2: return vk::Format::eR8G8Sint;
				case 3: return vk::Format::eR8G8B8Sint;
				case 4: return vk::Format::eR8G8B8A8Sint;
				default: return vk::Format::eUndefined;
			}
		}
		case spirv_cross::SPIRType::BaseType::UByte: {
			switch (rows) {
				case 1: return vk::Format::eR8Uint;
				case 2: return vk::Format::eR8G8Uint;
				case 3: return vk::Format::eR8G8B8Uint;
				case 4: return vk::Format::eR8G8B8A8Uint;
				default: return vk::Format::eUndefined;
			}
		}
		case spirv_cross::SPIRType::BaseType::Short: {
			switch (rows) {
				case 1: return vk::Format::eR16Sint;
				case 2: return vk::Format::eR16G16Sint;
				case 3: return vk::Format::eR16G16B16Sint;
				case 4: return vk::Format::eR16G16B16A16Sint;
				default: return vk::Format::eUndefined;
			}
		}
		case spirv_cross::SPIRType::BaseType::UShort: {
			switch (rows) {
				case 1: return vk::Format::eR16Uint;
				case 2: return vk::Format::eR16G16Uint;
				case 3: return vk::Format::eR16G16B16Uint;
				case 4: return vk::Format::eR16G16B16A16Uint;
				default: return vk::Format::eUndefined;
			}
		}
		case spirv_cross::SPIRType::BaseType::Int: {
			switch (rows) {
				case 1: return vk::Format::eR32Sint;
				case 2: return vk::Format::eR32G32Sint;
				case 3: return vk::Format::eR32G32B32Sint;
				case 4: return vk::Format::eR32G32B32A32Sint;
				default: return vk::Format::eUndefined;
			}
		}
		case spirv_cross::SPIRType::BaseType::UInt: {
			switch (rows) {
				case 1: return vk::Format::eR32Uint;
				case 2: return vk::Format::eR32G32Uint;
				case 3: return vk::Format::eR32G32B32Uint;
				case 4: return vk::Format::eR32G32B32A32Uint;
				default: return vk::Format::eUndefined;
			}
		}
		default:
			return vk::Format::eUndefined;
	}
}