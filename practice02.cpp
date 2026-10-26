#include <cstdio>
#include <memory>
#include <omp.h>
#include "include/course_options.h"

// Практика 2, 12.10.2026: ручное распределение работы между потоками.
// За основу взяты три последовательные версии из pr2.zip.
// Для сборки нужен флаг -fopenmp.

// В этом учебном примере предполагаем размер строки кеша 64 байта.
// Это не универсальная константа для всех процессоров.
#define CACHE_LINE 64
#define D (CACHE_LINE / sizeof(double))

// Версия 1 (sum_no_omp.cpp): каждый поток читает каждый T-й элемент.
// Частичные суммы соседних потоков находятся рядом в памяти.
double sum_par(const double* v, size_t n)
{
    int max_threads = omp_get_max_threads();
    auto partial_sums = std::make_unique<double[]>(max_threads);

    #pragma omp parallel
    {
        size_t t = omp_get_thread_num();
        size_t T = omp_get_num_threads();
        for (size_t i = t; i < n; i += T)
        {
            partial_sums[t] += v[i];
        }
    }

    double s = 0.0;
    for (int t = 0; t < max_threads; ++t)
    {
        s += partial_sums[t];
    }
    return s;
}

// Версия 2 (sum_no_omp_2.cpp): частичные суммы разнесены на 64 байта.
// Это устраняет ложное разделение строки кеша при записи сумм
// на процессоре со строкой кеша 64 байта. Чтение пока остаётся циклическим.
double sum_loc(const double* v, size_t n)
{
    int max_threads = omp_get_max_threads();
    auto partial_sums = std::make_unique<double[]>(max_threads * D);

    #pragma omp parallel
    {
        size_t t = omp_get_thread_num();
        size_t T = omp_get_num_threads();
        for (size_t i = t; i < n; i += T)
        {
            partial_sums[t * D] += v[i];
        }
    }

    double s = 0.0;
    for (int t = 0; t < max_threads; ++t)
    {
        s += partial_sums[t * D];
    }
    return s;
}

// Версия 3 (sum_no_omp_3.cpp, самая поздняя): непрерывные блоки
// массива и древовидное сложение частичных сумм.
double sum_blocks(const double* v, size_t n)
{
    int max_threads = omp_get_max_threads();
    auto partial_sums = std::make_unique<double[]>(max_threads * D);

    #pragma omp parallel
    {
        size_t t = omp_get_thread_num();
        size_t T = omp_get_num_threads();

        // d — число элементов потока, b — начало его блока.
        // Первые n % T потоков получают по одному лишнему элементу.
        size_t d = n / T;
        size_t b = n % T;
        if (t < n % T)
        {
            d += 1;
            b = d * t;
        }
        else
        {
            b += d * t;
        }
        size_t e = b + d;

        double my_sum = 0.0;
        for (size_t i = b; i < e; ++i)
        {
            my_sum += v[i];
        }
        partial_sums[t * D] = my_sum;

        // Все частичные суммы должны быть готовы до объединения.
        #pragma omp barrier
        for (size_t step = 1; step < T; step *= 2)
        {
            if (t % (2 * step) == 0 && t + step < T)
            {
                partial_sums[t * D] += partial_sums[(t + step) * D];
            }
            // Все потоки заканчивают текущий уровень дерева.
            #pragma omp barrier
        }
    }

    return partial_sums[0];
}

int main(int argc, char** argv)
{
    size_t N = 1u << 28;
    int status = course::read_options(argc, argv, N);
    if (status != 0) return status == 1 ? 0 : 1;
    auto data = std::make_unique<double[]>(N);
    for (size_t i = 0; i < N; ++i)
    {
        data[i] = i;
    }

    double expected = double(N) * double(N - 1) / 2; // При N=2^28: 2^55 - 2^27.

    auto t1 = omp_get_wtime();
    double s1 = sum_par(data.get(), N);
    auto t2 = omp_get_wtime();
    double s2 = sum_loc(data.get(), N);
    auto t3 = omp_get_wtime();
    double s3 = sum_blocks(data.get(), N);
    auto t4 = omp_get_wtime();

    printf("sum (cyclic): %.0f\n", s1);
    printf("sum (padded): %.0f\n", s2);
    printf("sum (blocks + tree): %.0f\n", s3);
    printf("expected: %.0f\n", expected);

    printf("\nError (cyclic): %.6e\n", (s1 - expected) / expected);
    printf("Error (padded): %.6e\n", (s2 - expected) / expected);
    printf("Error (blocks + tree): %.6e\n", (s3 - expected) / expected);

    printf("\nTime (cyclic): %.0f ms.\n", (t2 - t1) * 1000);
    printf("Time (padded): %.0f ms.\n", (t3 - t2) * 1000);
    printf("Time (blocks + tree): %.0f ms.\n", (t4 - t3) * 1000);
    return 0;
}
