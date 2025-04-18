//
// Created by radue on 4/16/2025.
//

export module math.constants;

import <glm/ext/scalar_constants.hpp>;
import <type_traits>;

namespace Coral::Math {
    export template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    constexpr T Pi() {
        return glm::pi<T>();
    }
}