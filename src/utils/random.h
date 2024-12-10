//
// Created by radue on 12/1/2024.
//

#pragma once
#include <random>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/normal_distribution.hpp>
#include <glm/glm.hpp>

namespace Utils {
    class Random {
    public:
        template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        static T UniformIntegralValue(T min, T max) {
            boost::random::uniform_int_distribution<T> dist(min, max);
            return dist(m_rng);
        }

        template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
        static T UniformRealValue(T min, T max) {
            boost::random::uniform_real_distribution<T> dist(min, max);
            return dist(m_rng);
        }

        template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
        static T NormalValue(T mean, T stddev) {
            boost::random::normal_distribution<T> dist(mean, stddev);
            return dist(m_rng);
        }

        template<glm::length_t N, typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        static glm::vec<N, T> UniformIntegralVector(const glm::vec<N, T> &min, const glm::vec<N, T> &max) {
            glm::vec<N, T> result;
            for (glm::length_t i = 0; i < N; i++) {
                result[i] = UniformIntegralValue(min[i], max[i]);
            }
            return result;
        }

        template<glm::length_t N, typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
        static glm::vec<N, T> UniformRealVector(const glm::vec<N, T> &min, const glm::vec<N, T> &max) {
            glm::vec<N, T> result;
            for (glm::length_t i = 0; i < N; i++) {
                result[i] = UniformRealValue(min[i], max[i]);
            }
            return result;
        }

        template<glm::length_t N, typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
        static glm::vec<N, T> NormalVector(const glm::vec<N, T> &mean, const glm::vec<N, T> &stddev) {
            glm::vec<N, T> result;
            for (glm::length_t i = 0; i < N; i++) {
                result[i] = NormalValue(mean[i], stddev[i]);
            }
            return result;
        }

        static glm::vec3 Color() {
            return glm::vec3(UniformRealValue(0.1f, .9f), UniformRealValue(0.1f, .9f), UniformRealValue(0.1f, .9f));
        }

        static glm::vec4 ColorWithAlpha() {
            return glm::vec4(Color(), UniformRealValue(0.0f, 1.0f));
        }

        static glm::vec3 Direction() {
            return glm::normalize(glm::vec3(UniformRealValue(-1.0f, 1.0f), UniformRealValue(-1.0f, 1.0f), UniformRealValue(-1.0f, 1.0f)));
        }

    private:
        static boost::random::mt19937 m_rng;
    };
}
