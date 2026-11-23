#include <cstdio>
#include <memory>
#include <new>
#include <omp.h>
#include <barrier>
#include <thread>
#include <vector>
#include <chrono>
#include <immintrin.h>

void matrix_add(const float *A, const float *B, float *C, size_t N) {
    for(size_t i=0;i<N;++i) {
        for(size_t j=0;j<N;++j) {
            C[i*N+j] = A[i*N+j] + B[i*N+j];
        }
    }
}

__attribute__((target("sse")))
void matrix_add_sse(const float* A, const float* B, float* C, size_t N)
{
    for (size_t i = 0; i < N; ++i) {
        size_t j = 0;
        for (; j + 4 <= N; j += 4) {
            __m128 a = _mm_loadu_ps(&A[i * N + j]);
            __m128 b = _mm_loadu_ps(&B[i * N + j]);
            _mm_storeu_ps(&C[i * N + j], _mm_add_ps(a, b));
        }
        // Остаток строки, если N не кратно 4.
        for (; j < N; ++j) {
            C[i * N + j] = A[i * N + j] + B[i * N + j];
        }
    }
}

__attribute__((target("avx512f")))
void matrix_add_16(const float* A, const float* B, float* C, size_t N)
{
    for(size_t i=0;i<N;++i) {
        size_t j = 0;
        for(;j + 16 <= N;j += 16) {
            __m512 a = _mm512_loadu_ps(&A[i*N+j]);
            __m512 b = _mm512_loadu_ps(&B[i*N+j]);
            _mm512_storeu_ps(&C[i*N+j], _mm512_add_ps(a, b));
        }
        // Остаток строки, если N не кратно 16.
        for(;j<N;++j) {
            C[i*N+j] = A[i*N+j] + B[i*N+j];
        }
    }
}
int main()
{
    const bool has_sse = __builtin_cpu_supports("sse");
    const bool has_avx512 = __builtin_cpu_supports("avx512f");

    size_t N = 1<<9;
    auto data = std::make_unique<float[]>(N*N);
    auto data2 = std::make_unique<float[]>(N*N);
    auto result = std::make_unique<float[]>(N*N);
    auto result4 = std::make_unique<float[]>(N*N);
    auto result16 = std::make_unique<float[]>(N*N);

    for (size_t i=0;i< N*N;++i) {
        data[i] = float(i);
        data2[i] = float(i * 2);
    }

    auto t1 = std::chrono::steady_clock::now();
    matrix_add(data.get(), data2.get(), result.get(), N);
    auto t2 = std::chrono::steady_clock::now();
    if (has_sse) {
        matrix_add_sse(data.get(), data2.get(), result4.get(), N);
    }
    auto t3 = std::chrono::steady_clock::now();
    if (has_avx512) {
        matrix_add_16(data.get(), data2.get(), result16.get(), N);
    }
    auto t4 = std::chrono::steady_clock::now();

    for (size_t i=0;i<N*N;++i) {
        float expected = data[i] + data2[i];
        if (result[i] != expected || (has_sse && result4[i] != expected)
            || (has_avx512 && result16[i] != expected)) {
            fprintf(stderr, "Failure at i = %zu\n", i);
            return 1;
        }
    }

    std::chrono::duration<double> elapsed = t2 - t1;
    std::chrono::duration<double> elapsed4 = t3 - t2;
    std::chrono::duration<double> elapsed16 = t4 - t3;
    printf("Check: OK\n");
    printf("Time (usual): %.3f ms\n", elapsed.count() * 1000);
    if (has_sse) {
        printf("Time (SSE, 4 floats): %.3f ms\n", elapsed4.count() * 1000);
        if (elapsed4.count() > 0) {
            printf("Speedup (SSE): %.2f x\n", elapsed.count() / elapsed4.count());
        }
    }
    if (has_avx512) {
        printf("Time (AVX-512, 16 floats): %.3f ms\n", elapsed16.count() * 1000);
        if (elapsed16.count() > 0) {
            printf("Speedup (AVX-512): %.2f x\n", elapsed.count() / elapsed16.count());
        }
    } else {
        printf("AVX-512F unavailable: SIMD method skipped.\n");
    }

    return 0;
}
