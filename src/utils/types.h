//
// Created by radue on 4/15/2025.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <imgui.h>

namespace spirv_cross {
	class SPIRType;
}

namespace vk {
	enum class Format;
}

namespace Coral {
    using UUID = boost::uuids::uuid;
    using UUIDGenerator = boost::uuids::random_generator;

    using String = std::string;
    using Path = std::filesystem::path;
    using Time = std::chrono::high_resolution_clock::time_point;

    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;
    using f32 = float;
    using f64 = double;
    using f128 = long double;

    using usize = std::size_t;
    using isize = std::ptrdiff_t;

	template<typename T> requires std::is_enum_v<T>
	struct EnumHash {
		std::size_t operator()(T value) const {
			return std::hash<std::underlying_type_t<T>>{}(static_cast<std::underlying_type_t<T>>(value));
		}
	};

	template <typename Key>
	using HashType = std::conditional_t<std::is_enum_v<Key>, EnumHash<Key>, std::hash<Key>>;

	template <typename Key, typename T>
	using UnorderedMap = std::unordered_map<Key, T, HashType<Key>>;

	template<typename T>
	using UnorderedSet = std::unordered_set<T, HashType<T>>;

    template<typename T> requires std::is_arithmetic_v<T>
    ImGuiDataType GetImGuiDataType() {
        if constexpr (std::is_same_v<T, u8>) {
            return ImGuiDataType_U8;
        } else if constexpr (std::is_same_v<T, u16>) {
            return ImGuiDataType_U16;
        } else if constexpr (std::is_same_v<T, u32>) {
            return ImGuiDataType_U32;
        } else if constexpr (std::is_same_v<T, u64>) {
            return ImGuiDataType_U64;
        } else if constexpr (std::is_same_v<T, i8>) {
            return ImGuiDataType_S8;
        } else if constexpr (std::is_same_v<T, i16>) {
            return ImGuiDataType_S16;
        } else if constexpr (std::is_same_v<T, i32>) {
            return ImGuiDataType_S32;
        } else if constexpr (std::is_same_v<T, i64>) {
            return ImGuiDataType_S64;
        } else if constexpr (std::is_same_v<T, f32>) {
            return ImGuiDataType_Float;
        } else if constexpr (std::is_same_v<T, f64>) {
            return ImGuiDataType_Double;
        } else {
            throw std::invalid_argument("Unsupported type for ImGuiDataType");
        }
    }

	vk::Format FormatFromString(const String& format);
	vk::Format SPIRTypeToVkFormatConverter(const spirv_cross::SPIRType& type);

}
