// УЛУЧШИЛИ ЛОКАЛЬНОСТЬ доступа

// d (кол-во элементов для расчёта) = n/t + 1 если t <= n mod T 
//                                    ИЛИ n/t если t > n mod T

// a (кол-во задач на поток) = ((n/t )+1)*t если t <= n mod T 
//                                  или
//                              ((n/t) +1) * n mod T + (n/t)*(t - n mod T) = (n/t) + n mod T если t > n mod T
//

#include <stdio.h>
#include <memory>
#include <omp.h>
#include <cstdlib>

// для компиляции нужно указать флаг -fopenmp

#define CACHE_LINE 64 // оптимальный размер кеша для всех процессоров

#define D CACHE_LINE/sizeof(double)


double sum_loc(const double* v, size_t n){
    double s = 0.0;
    
    // выделили память на omp_get_num_procs(вернуло число) элементов размера double в виде указателей
    double* partial_sums = (double*) malloc(omp_get_num_procs()*CACHE_LINE);

    #pragma omp parallel
    {
        unsigned t = omp_get_thread_num();
        unsigned T = omp_get_num_threads(); // возвращает количество потоков в пуле
        
        // этот кусок кода позволяет делить массив по элементам с распределением остатка от целого деление
        size_t d = n / T;
        size_t b = n % T;
        if(t < n % T)
        {
            d += 1;
            b = d*t;
        }
        else
        {
            b += d * t ;
        }
        // size_t d = (t < n % T) ? (n / T + 1) : (n / T);
        // size_t b = (t < n % T) ? ((n / T + 1) * t) : ((n / T) * t + (n % T));
        size_t e = b + d;
        // =============================================================================================

        double my_sum;
        for (size_t i = b; i < e; i+=1) {
            my_sum += v[i];
            
        }
        partial_sums[t*D] += my_sum;
        
        #pragma omp barrier
        // for (size_t i=0; i<T; ++i)
            // s+=partial_sums[i*CACHE_LINE/sizeof(double)];
        for(size_t s = 1, r = 2; s < T; s = r, r+=r){
            if (t % r == 0 && t + s < T){
                // auto D = CACHE_LINE/sizeof(double); // вынесем её из цикла, чтобы не считать каждый раз
                partial_sums[t * D] += partial_sums[(t + s)*D];
            }
            #pragma omp barrier
                
        }
    }
    s = partial_sums[0];
    free(partial_sums);
    return s;
}





int main(int argc, char ** argv){
    size_t N = 1u << 28;
    auto data = std::make_unique<double[]>(N);
    for (size_t i = 0; i < N; ++i)
        data[i] = i;

    double expected = 0x1P+55 - 0x1P+27; // 2^56 - 2^27

    auto t1 = omp_get_wtime();

    double s1 = sum_loc(data.get(), N);

    auto t2 = omp_get_wtime();

    printf("sum (small to large): %g\n", s1);

    printf("Error %g\n", s1-expected);

    printf("Time taken: %g ms.\n", t2-t1);

    return 0;
}