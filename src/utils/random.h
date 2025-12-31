//
// Created by radue on 12/1/2024.
//

#pragma once
#include <random>
#include <math/vector.h>

namespace Coral::Utils {
    class Random {
    public:
        template<typename T> requires std::is_integral_v<T>
        static T UniformIntegralValue(T min, T max) {
            std::uniform_int_distribution<T> dist(min, max);
            return dist(m_rng);
        }

        template<typename T> requires std::is_floating_point_v<T>
        static T UniformRealValue(T min, T max) {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(m_rng);
        }

        template<typename T> requires std::is_floating_point_v<T>
        static T NormalValue(T mean, T stddev) {
            std::normal_distribution<T> dist(mean, stddev);
            return dist(m_rng);
        }

        template<u8 N, typename T> requires std::is_integral_v<T>
        static Math::Vector<T, N> UniformIntegralVector(const Math::Vector<T, N> &min, const Math::Vector<T, N> &max) {
            Math::Vector<T, N> result;
            for (glm::length_t i = 0; i < N; i++) {
                result[i] = UniformIntegralValue(min[i], max[i]);
            }
            return result;
        }

        template<u8 N, typename T> requires std::is_floating_point_v<T>
        static Math::Vector<T, N> UniformRealVector(const Math::Vector<T, N> &min, const Math::Vector<T, N> &max) {
			Math::Vector<T, N> result;
            for (glm::length_t i = 0; i < N; i++) {
                result[i] = UniformRealValue(min[i], max[i]);
            }
            return result;
        }

        template<u8 N, typename T> requires std::is_floating_point_v<T>
        static Math::Vector<T, N> NormalVector(const Math::Vector<T, N> &mean, const Math::Vector<T, N> &stddev) {
            Math::Vector<T, N> result;
            for (glm::length_t i = 0; i < N; i++) {
                result[i] = NormalValue(mean[i], stddev[i]);
            }
            return result;
        }

   //      static Color Color() {
   //          return Coral::Color(
			// 	UniformRealValue(0.0f, 1.0f),
			// 	UniformRealValue(0.0f, 1.0f),
			// 	UniformRealValue(0.0f, 1.0f)
			// );
   //      }

    	template<u8 N> requires (N == 2 || N == 3)
		static Math::Vector<f32, N> Direction();

    private:
        inline static auto m_rng = std::mt19937(std::random_device()());
    };
}
