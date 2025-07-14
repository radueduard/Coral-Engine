//
// Created by radue on 13/07/2025.
//
export module math.matrix;
import math.vector;

import types;
import std;

import <glm/glm.hpp>;

namespace Coral::Math {
    export template<typename T, u32 N, u32 M> requires std::is_arithmetic_v<T> && (N > 1) && (M > 1)
    struct Matrix {
        constexpr Matrix() = default;

        constexpr explicit Matrix(T scalar) {
            for (int i = 0; i < std::min(N, M); ++i) {
                data[i][i] = scalar;
            }
        }

        explicit constexpr Matrix(const std::array<Vector<T, M>, N>& columns) {
            std::copy(columns.begin(), columns.end(), data);
        }

        template<typename... Args> requires (sizeof...(Args) <= N * M) && (std::is_same_v<T, Args> && ...)
        explicit constexpr Matrix(Args... values) {
            int i = 0, j = 0;
            for (const auto& value : { values... }) {
                data[i][j] = value;
                ++i;
                if (i == N) {
                    i = 0;
                    ++j;
                    if (j == M) break;
                }
            }
        }

        constexpr Vector<T, M>& operator[](int index) {
            return data[index];
        }

        constexpr const Vector<T, M>& operator[](int index) const {
            return data[index];
        }

        // Equality and comparison operators

        constexpr bool operator==(const Matrix& other) const {
            for (int i = 0; i < N; ++i) {
                if (data[i] != other[i]) {
                    return false;
                }
            }
            return true;
        }

        constexpr bool operator!=(const Matrix& other) const {
            return !(*this == other);
        }

        // Arithmetic operations

        constexpr Matrix<T, M, N> Transpose() const {
            Matrix<T, M, N> result;
            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < M; ++j) {
                    result[j][i] = data[i][j];
                }
            }
            return result;
        }

        constexpr T Determinant() const requires (N == M) {
            return glm::determinant(*reinterpret_cast<const glm::mat<N, M, T>*>(this));
        }

        constexpr Matrix Inverse() const requires (N == M) {
            glm::mat<N, M, T> glmInverse = glm::inverse(*reinterpret_cast<const glm::mat<N, M, T>*>(this));
            return *reinterpret_cast<Matrix*>(&glmInverse);
        }

        constexpr Matrix operator-() const {
            Matrix result;
            for (int i = 0; i < N; ++i) {
                result[i] = -data[i];
            }
            return result;
        }

        constexpr Matrix operator+(const Matrix& other) const {
            Matrix result;
            for (int i = 0; i < N; ++i) {
                result[i] = data[i] + other[i];
            }
            return result;
        }
        constexpr Matrix operator+(T scalar) const {
            Matrix result;
            for (int i = 0; i < N; ++i) {
                result[i] = data[i] + scalar;
            }
            return result;
        }

        constexpr Matrix& operator+=(const Matrix& other) {
            for (int i = 0; i < N; ++i) {
                data[i] += other[i];
            }
            return *this;
        }

        constexpr Matrix& operator+=(T scalar) {
            for (int i = 0; i < N; ++i) {
                data[i] += scalar;
            }
            return *this;
        }

        constexpr Matrix operator-(const Matrix& other) const {
            Matrix result;
            for (int i = 0; i < N; ++i) {
                result[i] = data[i] - other[i];
            }
            return result;
        }

        constexpr Matrix operator-(T scalar) const {
            Matrix result;
            for (int i = 0; i < N; ++i) {
                result[i] = data[i] - scalar;
            }
            return result;
        }

        constexpr Matrix& operator-=(const Matrix& other) {
            for (int i = 0; i < N; ++i) {
                data[i] -= other[i];
            }
            return *this;
        }

        constexpr Matrix& operator-=(T scalar) {
            for (int i = 0; i < N; ++i) {
                data[i] -= scalar;
            }
            return *this;
        }

        template<int P, int Q> requires (M == P) && (Q > 0)
        constexpr Matrix operator*(const Matrix<T, P, Q>& other) const {
            Matrix<T, N, Q> result;
            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < P; ++j) {
                    result[i][j] = 0;
                    for (int k = 0; k < M; ++k) {
                        result[i][j] += data[i][k] * other[k][j];
                    }
                }
            }
            return result;
        }

        template<int P> requires (M == P)
        constexpr Vector<T, P> operator*(const Vector<T, P>& vector) const {
            Vector<T, P> result;
            for (int i = 0; i < N; ++i) {
                result[i] = 0;
                for (int j = 0; j < M; ++j) {
                    result[i] += data[i][j] * vector[j];
                }
            }
            return result;
        }

        constexpr Matrix operator*(T scalar) const {
            Matrix result;
            for (int i = 0; i < N; ++i) {
                result[i] = data[i] * scalar;
            }
            return result;
        }

        constexpr Matrix& operator*=(const Matrix& other) {
            *this = *this * other;
            return *this;
        }

        constexpr Matrix& operator*=(T scalar) {
            for (int i = 0; i < N; ++i) {
                data[i] *= scalar;
            }
            return *this;
        }

        constexpr Matrix operator-() {
            Matrix result;
            for (int i = 0; i < N; ++i) {
                result[i] = -data[i];
            }
            return result;
        }

        template <u32 O = N, u32 P = M> requires (O == P)
        constexpr Matrix operator/(const Matrix& other) const {
            return *this * other.Inverse();
        }

        template <u32 O = N, u32 P = M> requires (O == P)
        constexpr static Matrix Identity() {
            return Matrix(1.0f);
        }

        // Conversion operators and constructors

        constexpr explicit Matrix(const glm::mat<N, M, T>& matrix) {
            std::copy(&matrix[0][0], &matrix[0][0] + N * M, &data[0][0]);
        }

        explicit constexpr operator glm::mat<N, M, T>() const {
            glm::mat<N, M, T> result;
            std::copy(data, &data[N], &result[0]);
            return result;
        }

    private:
        std::array<Vector<T, M>, N> data {};
    };

    export {
        template <typename T> requires std::is_arithmetic_v<T>
        using Matrix2 = Matrix<T, 2, 2>;

        template <typename T> requires std::is_arithmetic_v<T>
        using Matrix3 = Matrix<T, 3, 3>;

        template <typename T> requires std::is_arithmetic_v<T>
        using Matrix4 = Matrix<T, 4, 4>;

        using Matrix2f = Matrix<f32, 2, 2>;
        using Matrix3f = Matrix<f32, 3, 3>;
        using Matrix4f = Matrix<f32, 4, 4>;
    }
}

template<typename T, u8 N, u8 M>
struct std::formatter<Coral::Math::Matrix<T, N, M>> : std::formatter<T> {
    template<typename FormatContext>
    auto format(const Coral::Math::Matrix<T, N, M>& matrix, FormatContext& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "[");
        for (u8 i = 0; i < M; ++i) {
            if (i > 0) {
                out = std::format_to(out, ", ");
            }
            out = std::format_to(out, "[");
            for (u8 j = 0; j < N; ++j) {
                if (j > 0) {
                    out = std::format_to(out, ", ");
                }
                out = std::formatter<T>::format(matrix[j][i], ctx);
            }
            out = std::format_to(out, "]");
        }
        out = std::format_to(out, "]");
        return out;
    }
};