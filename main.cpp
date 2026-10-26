#include <algorithm>
#include <iostream>
#include <cstdio>
#include <numeric>
#include <vector>
#include <stdlib.h>
#include<memory>
#include <omp.h>
#include "include/course_options.h"

double sum(const double* V, size_t n)
{
    double a = 0;
    
    for (size_t i = 0; i < n; ++i)
    {
        a += V[i];
    }

    return a;
}
// сложение от большего элемента к меньшему.
double sum_reverse(const double* V, size_t n)
{
    double a = 0;

    for (size_t i = n; i > 0; --i)
    {
        a += V[i - 1];
    }

    return a;
}

double sum_parallel(const double* V, size_t n)
{
    double a = 0;

    #pragma omp parallel for reduction(+:a)
    for (size_t i = 0; i < n; ++i)
    {
        a += V[i];
    }

    return a;
}


int main(int argc, char** argv)
{
    size_t N = 1 << 28;
    int status = course::read_options(argc, argv, N);
    if (status != 0) return status == 1 ? 0 : 1;
    auto data = std::make_unique<double[]>(N);

    for (size_t i = 0; i < N; ++i)
    {
        data[i] = i;
    }

    double expected = double(N) * double(N - 1) / 2; // При N=2^28: 2^55 - 2^27.

    auto t1 = omp_get_wtime();
    double s1 = sum(data.get(), N);
    auto t2 = omp_get_wtime();
    double s_reverse = sum_reverse(data.get(), N);
    auto t3 = omp_get_wtime();
    double s2 = sum_parallel(data.get(), N);
    auto t4 = omp_get_wtime();

    double serial_ms = (t2 - t1) * 1000;
    double reverse_ms = (t3 - t2) * 1000;
    double parallel_ms = (t4 - t3) * 1000;

    printf("sum (ascending): %.0f\n", s1);
    printf("sum (descending): %.0f\n", s_reverse);
    printf("sum (parallel): %.0f\n", s2);
    printf("expected: %.0f\n", expected);

    printf("\nError (ascending): %.6e\n", (s1 - expected)/expected);
    printf("Error (descending): %.6e\n", (s_reverse - expected)/expected);
    printf("Error (parallel): %.6e\n", (s2 - expected)/expected);

    printf("\nTime (ascending): %.0f ms.\n", serial_ms);
    printf("Time (descending): %.0f ms.\n", reverse_ms);
    printf("Time (parallel): %.0f ms.\n", parallel_ms);
    return 0;
    
}
// с помощью Open MP нужно распалерить функцию sum, чтобы ускорить вычисление суммы элементов массива
// преподаватель говорит, что нужно вообще в Microsoft Visual Studio, а хочу остаться в VSCode. И также еще проблемы могут быть с компилятором.

//Последовательное сложение от меньшего к большему
//Последовательное сложение
//Последовательное от большего к меньшему
//Нужно вывести это все рамках одной программы, чтобы сделать выводы.
