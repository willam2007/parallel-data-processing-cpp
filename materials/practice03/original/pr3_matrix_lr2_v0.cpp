// SSE...SSE 4.2 нужен будет для второй ЛР. (128 бит - 16 байт) 
// sse: __m128 _mm_add_ps (__m128 x, __mm128 y); // для одинарной точности
// sse: __m128d _mm_add_pd (__m128d x, __mm128d y); // для double
// есть ещё _mm_add_epi32(__m128i x, __m128i y);
// для AVX
// __m256 _mm256_add_ps(__m256 x, __m256 y);
// для AVX-512
// __m512 _mm512_add_ps(__m256 x, __m256 y);
#include <thread>
#include <emmintrin.h>
#include <immintrin.h>
#include <xmmintrin.h> // Заголовочный файл для SSE интринсик
#include <chrono>
#include <stdio.h>

using namespace std::chrono;

void matrix_add(float* A,const float* B, const float* C, size_t N){
    // Классическая реализация сложения матриц
    for(size_t i = 0; i < N; ++i){
        for(size_t j = 0; j < N; ++j){
            A[N*i + j] = B[N*i + j] + C[N*i + j];
        }
    }
}

// void matrix_add_avx512(float* A,const float* B, const float* C, size_t N){
//     // Сложение матриц на AVX-512

//     for(size_t i = 0; i < N; ++i){
//         for(size_t j = 0; j < N / 16; ++j){
//             //A[N*i + j] = B[N*i + j] + C[N*i + j];
//             __m512 b = _mm512_loadu_ps(&B[N*i+j*16]);
//             __m512 c = _mm512_loadu_ps(&C[N*i+j*16]);
//             _mm512_storeu_ps(&A[N*i+j*16],
//                 _mm512_add_ps(b, c));
//         }
//     }
// }


void matrix_add_sse(float* A, const float* B, const float* C, size_t N) {
    // Сложение матриц на SSE
    for (size_t i = 0; i < N; ++i) {
        // Делим на 4, так как один регистр SSE (__m128) вмещает 4 значения float
        for (size_t j = 0; j < N / 4; ++j) {
            size_t index = N * i + j * 4;

            __m128 b = _mm_loadu_ps(&B[index]);
            __m128 c = _mm_loadu_ps(&C[index]);
            
            _mm_storeu_ps(&A[index], _mm_add_ps(b, c));
        }
    }
}


void matrix_multiplication(float* C, size_t nc, size_t nr, size_t n, const float* A, const float* B){
    for (size_t c = 0; c < nc; ++c)
        for (size_t r=0;r<nr;++r){
            C[c*nr+r] = 0;
            for(size_t i=0; i<n;++i)
                C[c*nr+r] += A[i*nr+r] * B[c*n+i];
        }
}

void matrix_multiplication_avx512(){
    
}


void show_martix(const float* A, size_t nc, size_t nr){
    for (size_t i = 0; i < nr; ++i){
        for(size_t j = 0; j < nc; ++j)
            printf("%10.1f", A[j*nr + i]);
        printf("\n");
    }
}


int main(int argc, char ** argv){
    // size_t N = 1 << 12;
    // auto data = std::make_unique<float[]>(N*N),
        // data2 = std::make_unique<float[]>(N*N);
    // for (size_t i = 0; i < N*N; ++i)
    // {
    //     data[i] = i;
    // }

    // double expected = 0x1P+55 - 0x1P+27; // 2^56 - 2^27

    float A[2][3] = {
        {1, 3, -1},
        {2, 4, -2},
    };

    float B[4][2] = {
        {3, 0},
        {2, 1},
        {1, 2},
        {0, 3},
        
    };

    float C[4][3];

    auto t1 = std::chrono::steady_clock::now();

    matrix_multiplication((float*) C, 4, 3, 2,  & A[0][0], & B[0][0]);
    show_martix((float*) C, 4, 3);
    // matrix_add(data2.get(), data.get(), data.get(), N);

    auto t2 = steady_clock::now();

    // for (size_t i = 0; i < N*N; ++i){
    //     if (data2[i] != i+i){
    //         fprintf(stderr, "Failure at i = %i\n", (unsigned) i);
    //     }
    // }

    // printf("sum (small to large): %g\n", s1);

    // printf("Error %g\n", s1-expected);

    printf("Time taken: %u ms.\n", duration_cast<milliseconds>(t2-t1).count());

    return 0;
}

