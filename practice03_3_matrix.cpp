#include <algorithm>
#include <cstddef>
#include <cstdio>

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

// A имеет nr строк и n столбцов, B — n строк и nc столбцов.
// Матрицы хранятся по столбцам: элемент (r, c) лежит по индексу c * nr + r.
void matrix_multiplication(float* C, size_t nc, size_t nr, size_t n,
                           const float* A, const float* B)
{
    for (size_t c = 0; c < nc; ++c) {
        for (size_t r = 0; r < nr; ++r) {
            float sum = 0;
            for (size_t i = 0; i < n; ++i) {
                sum += A[i * nr + r] * B[c * n + i];
            }
            C[c * nr + r] = sum;
        }
    }
}

#if defined(__x86_64__) || defined(__i386__)
__attribute__((target("avx512f")))
void matrix_multiplication_avx512(float* C, size_t nc, size_t nr, size_t n,
                                  const float* A, const float* B)
{
    for (size_t c = 0; c < nc; ++c) {
        size_t r = 0;
        for (; r + 16 <= nr; r += 16) {
            __m512 sum = _mm512_setzero_ps();
            for (size_t i = 0; i < n; ++i) {
                // 16 соседних строк A умножаем на один и тот же элемент B.
                __m512 a = _mm512_loadu_ps(&A[i * nr + r]);
                __m512 b = _mm512_set1_ps(B[c * n + i]);
                sum = _mm512_add_ps(sum, _mm512_mul_ps(a, b));
            }
            _mm512_storeu_ps(&C[c * nr + r], sum);
        }
        for (; r < nr; ++r) {
            float sum = 0;
            for (size_t i = 0; i < n; ++i) {
                sum += A[i * nr + r] * B[c * n + i];
            }
            C[c * nr + r] = sum;
        }
    }
}
#endif

// P[c] задаёт строку ненулевого элемента столбца c. P — перестановка 0..N-1.
// При value != 0 обратная матрица получается транспонированием и заменой value на 1/value.
void permutation_and_inverse(float* A, float* B, const size_t* P, size_t N, float value)
{
    std::fill_n(A, N * N, 0.0f);
    std::fill_n(B, N * N, 0.0f);
    for (size_t c = 0; c < N; ++c) {
        A[c * N + P[c]] = value;
        B[P[c] * N + c] = 1.0f / value;
    }
}

void show_matrix(const float* A, size_t nc, size_t nr)
{
    for (size_t r = 0; r < nr; ++r) {
        for (size_t c = 0; c < nc; ++c) {
            std::printf("%6.1f", A[c * nr + r]);
        }
        std::printf("\n");
    }
}

int main()
{
    // Каждый столбец записан на отдельной строке инициализатора.
    const float A[] = {1, 3, -1,
                       2, 4, -2};
    const float B[] = {3, 0,
                       2, 1,
                       1, 2,
                       0, 3};
    const float expected[] = {3, 9, -3, 4, 10, -4, 5, 11, -5, 6, 12, -6};
    float C[12] = {};
    matrix_multiplication(C, 4, 3, 2, A, B);
    if (!std::equal(expected, expected + 12, C)) {
        std::fprintf(stderr, "Rectangular multiplication failed.\n");
        return 1;
    }
    std::printf("A * B (3 x 4):\n");
    show_matrix(C, 4, 3);

    const size_t P[] = {15, 1, 2, 10, 4, 5, 6, 7, 8, 9, 3, 11, 12, 13, 14, 0};
    const size_t N = sizeof(P) / sizeof(P[0]);
    float permutation[N * N];
    float inverse[N * N];
    float product[N * N];
    float identity[N * N] = {};
    for (size_t i = 0; i < N; ++i) {
        identity[i * N + i] = 1;
    }

    permutation_and_inverse(permutation, inverse, P, N, 0.5f);
    std::fill_n(product, N * N, -1.0f);
    matrix_multiplication(product, N, N, N, permutation, inverse);
    if (!std::equal(identity, identity + N * N, product)) {
        std::fprintf(stderr, "Permutation times inverse differs from identity.\n");
        return 1;
    }
    std::printf("Permutation * inverse: identity OK\n");

#if defined(__x86_64__) || defined(__i386__)
    if (__builtin_cpu_supports("avx512f")) {
        std::fill_n(product, N * N, -1.0f);
        matrix_multiplication_avx512(product, N, N, N, permutation, inverse);
        if (!std::equal(identity, identity + N * N, product)) {
            std::fprintf(stderr, "AVX-512 result differs from identity.\n");
            return 1;
        }
        std::printf("AVX-512: identity OK\n");
        return 0;
    }
#endif
    std::printf("AVX-512F unavailable: SIMD check skipped.\n");
    return 0;
}
