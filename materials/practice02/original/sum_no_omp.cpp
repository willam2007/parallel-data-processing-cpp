#include <stdio.h>
#include <memory>
#include <omp.h>
#include <cstdlib>

// для компиляции нужно указать флаг -fopenmp

#define CACHE_LINE 64


double sum_par(const double* v, size_t n){
    double s = 0.0;
    
    // выделили память на omp_get_num_procs(вернуло число) элементов размера double в виде указателей
    double* partial_sums = (double*) calloc(omp_get_num_procs(), sizeof(double));

    #pragma omp parallel
    {
        unsigned t = omp_get_thread_num();
        unsigned T = omp_get_num_threads(); // возвращает количество потоков в пуле
        for (size_t i = t; i < n; i+=T) {
            partial_sums[t] += v[i];
        }
    }
    
    for (size_t i=0; i<omp_get_num_procs(); ++i)
        s+=partial_sums[i];
    return s;
}





int main(int argc, char ** argv){
    size_t N = 1u << 28;
    auto data = std::make_unique<double[]>(N);
    for (size_t i = 0; i < N; ++i)
        data[i] = i;

    double expected = 0x1P+55 - 0x1P+27; // 2^56 - 2^27

    auto t1 = omp_get_wtime();

    double s1 = sum_par(data.get(), N);

    auto t2 = omp_get_wtime();

    printf("sum (small to large): %g\n", s1);

    printf("Error %g\n", s1-expected);

    printf("Time taken: %g ms.\n", t2-t1);

    return 0;
}