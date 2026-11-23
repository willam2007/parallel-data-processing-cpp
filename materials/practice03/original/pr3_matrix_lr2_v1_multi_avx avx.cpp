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

#define PER_ZMM (sizeof(__m128)/sizeof(X))

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

void matrix_multiplication_avx512(float* C, size_t nc, size_t nr, size_t r, const float* A, const float* B){
    for (size_t c = 0; c < nc; ++c){
        for(size_t r = 0; r < (nr / PER_ZMM(float)); ++r){

        }
    }
}


void show_martix(const float* A, size_t nc, size_t nr){
    for (size_t i = 0; i < nr; ++i){
        for(size_t j = 0; j < nc; ++j)
            printf("%10.1f", A[j*nr + i]);
        printf("\n");
    }
}


void obrat_matrix(float * M, int N, float* edinici_v_stroke){
    for (size_t i = 0; i < N; ++i)
    {
        int index = edinici_v_stroke[i];
        for (size_t j = 0; j < N; ++j)
            M[N*i + j] = j==index? 1 : 0;
    }
        
}

void perestav_obrat_matrix(float *C, float *A, float *B, size_t *P, int N, float value){
    for (size_t i = 0; i < N; ++i){
        A[N * i + P[i]] = value;
        B[P[i]*N+i] = 1 / value;
    }
}


int main(int argc, char ** argv){

    auto t1 = std::chrono::steady_clock::now();


    size_t P[16] = {15, 1, 2, 10, 4, 5, 6, 7,8, 9, 3, 11, 12, 13, 14, 0};
    int side = sizeof(P) / sizeof(P[0]);
    int n_size = side * side;
    float *AA = (float *) calloc(n_size, sizeof(float)),
        *BB = (float *) calloc(n_size, sizeof(float)),
        *CC = (float *) malloc(n_size * sizeof(float));
    
    perestav_obrat_matrix((float *) CC, (float *) AA, (float *) BB, (size_t *) P, side, 0.5);
    matrix_multiplication(CC, side, side, side, AA, BB);
    show_martix(CC, side, side);


    auto t2 = steady_clock::now();

    printf("Time taken: %u ms.\n", duration_cast<milliseconds>(t2-t1).count());

    return 0;
}

