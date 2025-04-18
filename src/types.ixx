//
// Created by radue on 4/15/2025.
//

export module types;

import <chrono>;
import <cstdint>;
import <filesystem>;
import <boost/uuid/uuid.hpp>;

namespace Coral {
    export using UUID = boost::uuids::uuid;
    export using String = std::string;
    export using Path = std::filesystem::path;
    export using Time = std::chrono::high_resolution_clock::time_point;

    export using u8 = uint8_t;
    export using u16 = uint16_t;
    export using u32 = uint32_t;
    export using u64 = uint64_t;
    export using i8 = int8_t;
    export using i16 = int16_t;
    export using i32 = int32_t;
    export using i64 = int64_t;
    export using f32 = float;
    export using f64 = double;
    export using f128 = long double;

    export using usize = std::size_t;
    export using isize = std::ptrdiff_t;
}