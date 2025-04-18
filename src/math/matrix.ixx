//
// Created by radue on 4/17/2025.
//

export module math.matrix;

import <glm/matrix.hpp>;

import types;
import math.vector;
import std;

namespace Coral::Math {
    export template<typename T, u32 N, u32 M> requires std::is_arithmetic_v<T> && (N > 0) && (M > 0)
    struct Matrix {
        using ValueType = T;
        using Column = Vector<T, M>;
        using Row = Vector<T, N>;
        using Type = Matrix<T, N, M>;
        using TransposeType = Matrix<T, M, N>;

        Matrix() = default;

        template<typename = std::enable_if_t<(N == M)>>
        constexpr explicit Matrix(T scalar) {
            for (int i = 0; i < N; ++i) {
                data[i][i] = scalar;
            }
        }

        constexpr explicit Matrix(const glm::mat<N, M, T>& matrix) {
            memcpy(data, &matrix, sizeof(matrix));
        }

        constexpr Matrix(const std::initializer_list<ValueType>& values) {
            int i = 0, j = 0;
            for (const auto& value : values) {
                data[i][j] = value;
                ++i;
                if (i == N) {
                    i = 0;
                    ++j;
                    if (j == M) break;
                }
            }
        }


        constexpr Matrix(const std::initializer_list<Column>& columns) {
            std::copy(columns.begin(), columns.end(), data);
        }

        template<typename = std::enable_if_t<(N != M)>>
        constexpr Matrix(const std::initializer_list<Row>& rows) {
            int r = 0;
            for (const auto& row : rows) {
                for (int c = 0; c < N; ++c) {
                    data[c][r] = row[c];
                }
                ++r;
                if (r == M) break;
            }
        }

        constexpr Column& operator[](int index) {
            if (index < 0 || index >= N) {
                throw std::out_of_range("Index out of range");
            }
            return data[index];
        }

        constexpr const Column& operator[](int index) const {
            if (index < 0 || index >= N) {
                throw std::out_of_range("Index out of range");
            }
            return data[index];
        }

        constexpr TransposeType Transpose() const {
            TransposeType result;
            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < M; ++j) {
                    result[j][i] = data[i][j];
                }
            }
            return result;
        }

        template<typename = std::enable_if_t<(N == M)>>
        constexpr T Determinant() const {
            return glm::determinant(*reinterpret_cast<const glm::mat<N, M, T>*>(this));
        }


        template<typename = std::enable_if_t<(N == M)>>
        constexpr Matrix Inverse() const {
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

        constexpr Matrix operator+(const Type& other) const {
            Matrix result;
            for (int i = 0; i < N; ++i) {
                result[i] = data[i] + other[i];
            }
            return result;
        }
        constexpr Matrix operator+(ValueType scalar) const {
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
        constexpr Matrix& operator+=(ValueType scalar) {
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
        constexpr Matrix operator-(ValueType scalar) const {
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
        constexpr Matrix& operator-=(ValueType scalar) {
            for (int i = 0; i < N; ++i) {
                data[i] -= scalar;
            }
            return *this;
        }

        template<int P, int Q, typename = std::enable_if_t<(M == P)>, typename = std::enable_if_t<(Q > 0)>>
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
        constexpr Matrix operator*(ValueType scalar) const {
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
        constexpr Matrix& operator*=(ValueType scalar) {
            for (int i = 0; i < N; ++i) {
                data[i] *= scalar;
            }
            return *this;
        }


        constexpr operator glm::mat<N, M, T>() const {
            glm::mat<N, M, T> result;
            std::copy(data, &data[N], &result[0]);
            return result;
        }



    private:
        Column data[N];
    };

    export template <typename T, u32 N, u32 M> requires std::is_arithmetic_v<T> && (N > 0) && (M > 0)
    std::ostream& operator<<(std::ostream& os, const Matrix<T, N, M>& mat) {
        os << "(";
        for (int j = 0; j < M; ++j) {
            os << "(";
            for (int i = 0; i < N; ++i) {
                os << mat[i][j];
                if (i < N - 1) {
                    os << ", ";
                }
            }
            os << ")";
            if (j < M - 1) {
                os << ",\n ";
            }
        }
        os << ")";
        return os;
    }

    export template <typename T> requires std::is_arithmetic_v<T>
    using Matrix2 = Matrix<T, 2, 2>;
    export template <typename T> requires std::is_arithmetic_v<T>
    using Matrix3 = Matrix<T, 3, 3>;
    export template <typename T> requires std::is_arithmetic_v<T>
    using Matrix4 = Matrix<T, 4, 4>;

}