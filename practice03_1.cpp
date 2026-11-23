#include <cstdio>
#include <memory>
#include <new>
#include <omp.h>
#include <barrier>
#include <thread>
#include <vector>
#include <chrono>

// Структура хранит частичную сумму одного потока.
// alignas разносит элементы массива p в памяти, чтобы уменьшить ложное
// разделение кеша: соседние потоки не мешают друг другу при записи своих сумм.
struct alignas(std::hardware_destructive_interference_size) p_t {
    double v; // Сумма элементов блока, который обрабатывает один поток.
};

// Функция предназначена для параллельного сложения n элементов массива v.
// Используем std::thread — потоки C++, вместо директив OpenMP.
// Каждый поток складывает свой непрерывный блок и сохраняет сумму в p[t].v.
// Затем частичные суммы объединяются деревом, с ожиданием через барьер bar.
// После завершения всех потоков функция должна вернуть общую сумму.
// Реализация пока дописывается на занятии.
double sum_loc_cpp(const double* v, size_t n)
{
    unsigned T = std::thread::hardware_concurrency();
    auto p = std::make_unique<p_t[]>(T);
    double a = 0;
    std::barrier<> bar{T};
    //пошла лямба функция
    auto worker_proc = [T, v, n, &p, &bar](unsigned t) {
        size_t s = n / T, b = n % T;
        if (t < b) {
            b = ++s * t;
        } else {
            b += s * t;
        }
        size_t e = b + s;
        p[t].v = 0;
        for (size_t i = b; i < e; ++i) {
            p[t].v += v[i];
        }
        bar.arrive_and_wait();
        for (size_t s = 1, r = 2; s < T; s = r, r += r) {
            if (t + s < T && t % r ==0)
                p[t].v += p[t + s].v;
            bar.arrive_and_wait();
        }
    };
    std::vector<std::thread> workers(T - 1);
    for (unsigned t = 1; t < T; ++t) {
        workers[t-1] = std::thread(worker_proc, t);
    }
    worker_proc(0); // главный поток выполняет работу потока 0.
    for (auto& w : workers) {
        w.join();
    }
    return p[0].v;
}

int main()
{
    size_t n = 1 << 28;
    auto data = std::make_unique<double[]>(n);
    for (size_t i = 0; i < n; ++i) {
        data[i] = i;
    }
    const double* v = data.get();
    using namespace std::chrono;
    auto t1 = steady_clock::now();
    auto result = sum_loc_cpp(v, n);
    auto t2 = steady_clock::now();
    printf("Time %llu ms\n",duration_cast<milliseconds>(t2 - t1).count());
}
//раунд дроби - реализовать самостоятельно, не особо должна отлчичаться 
//итого 4 реализации образуют первую работу.
// вторая работа будет - операции матриц
