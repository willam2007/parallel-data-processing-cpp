#include <cstdio>
#include <cstddef>
#include <cmath>
#include <memory>
#include <omp.h>
#include "include/course_options.h"

// Подготовка к практике 3, 26.10.2026. Точное задание преподавателя пока неизвестно.
// Здесь квадратные матрицы N x N. Элемент [i][j] хранится как A[i * N + j].
// В main() выбери task: 0 — сумма элементов A, 1 — A + B, 2 — A * x, 3 — A * B.

// 0. Прямое продолжение первой практики: сумма всех элементов матрицы.
double matrix_sum(const double* A, size_t N)
{
    double s = 0.0;
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            s += A[i * N + j];
        }
    }
    return s;
}

double matrix_sum_parallel(const double* A, size_t N)
{
    double s = 0.0;
    #pragma omp parallel for reduction(+:s) schedule(static)
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            s += A[i * N + j];
        }
    }
    return s;
}

// 1. Сложение матриц: C[i][j] = A[i][j] + B[i][j].
void matrix_add(const double* A, const double* B, double* C, size_t N)
{
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            C[i * N + j] = A[i * N + j] + B[i * N + j];
        }
    }
}

void matrix_add_parallel(const double* A, const double* B, double* C, size_t N)
{
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            C[i * N + j] = A[i * N + j] + B[i * N + j];
        }
    }
}

// 2. Матрица на вектор: y[i] — сумма произведений строки A и вектора x.
void matrix_vector(const double* A, const double* x, double* y, size_t N)
{
    for (size_t i = 0; i < N; ++i)
    {
        double s = 0.0;
        for (size_t j = 0; j < N; ++j)
        {
            s += A[i * N + j] * x[j];
        }
        y[i] = s;
    }
}

void matrix_vector_parallel(const double* A, const double* x, double* y, size_t N)
{
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < N; ++i)
    {
        // s объявлена внутри тела цикла: у каждого потока своя переменная.
        // Разные итерации записывают разные y[i], поэтому reduction не нужен.
        double s = 0.0;
        for (size_t j = 0; j < N; ++j)
        {
            s += A[i * N + j] * x[j];
        }
        y[i] = s;
    }
}

// Те же вычисления, но чтение матрицы идёт по столбцам, с шагом N.
// Это ПОСЛЕДОВАТЕЛЬНЫЙ вариант для сравнения локальности памяти.
void matrix_vector_columns(const double* A, const double* x, double* y, size_t N)
{
    for (size_t i = 0; i < N; ++i)
    {
        y[i] = 0.0;
    }
    for (size_t j = 0; j < N; ++j)
    {
        for (size_t i = 0; i < N; ++i)
        {
            y[i] += A[i * N + j] * x[j];
        }
    }
    // Нельзя просто распараллелить здесь внешний цикл по j:
    // разные потоки начнут одновременно изменять одни и те же y[i].
}

// 3. Матрица на матрицу: C[i][j] = сумма A[i][k] * B[k][j] по k.
void matrix_multiply(const double* A, const double* B, double* C, size_t N)
{
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            double s = 0.0;
            for (size_t k = 0; k < N; ++k)
            {
                s += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = s;
        }
    }
}

void matrix_multiply_parallel(const double* A, const double* B, double* C, size_t N)
{
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            double s = 0.0;
            for (size_t k = 0; k < N; ++k)
            {
                s += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = s;
        }
    }
}

// Дополнительный шаг из темы второй практики: порядок циклов i-k-j.
// Внутренний цикл читает B по строке, а не по столбцу.
void matrix_multiply_local(const double* A, const double* B, double* C, size_t N)
{
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < N; ++i)
    {
        // Обнуляем строку перед +=, в том числе при повторном вызове функции.
        for (size_t j = 0; j < N; ++j)
        {
            C[i * N + j] = 0.0;
        }
        for (size_t k = 0; k < N; ++k)
        {
            double a = A[i * N + k];
            for (size_t j = 0; j < N; ++j)
            {
                C[i * N + j] += a * B[k * N + j];
            }
        }
    }
}

// Сравниваем все элементы результатов, а не только их общую сумму.
double max_error(const double* reference, const double* result, size_t count)
{
    double largest = 0.0;
    for (size_t i = 0; i < count; ++i)
    {
        double error = std::fabs(reference[i] - result[i]);
        if (!std::isfinite(error))
        {
            return error;
        }
        if (error > largest)
        {
            largest = error;
        }
    }
    return largest;
}

int main(int argc, char** argv)
{
    // Меняй эту строку на паре: 0 — сумма элементов, 1 — сложение матриц,
    // 2 — матрица * вектор, 3 — матрица * матрица.
    int task = 2;
    if (task < 0 || task > 3)
    {
        printf("Choose task 0, 1, 2 or 3.\n");
        return 1;
    }

    // Параметры командной строки позволяют запускать на другом ПК без правки кода.
    size_t N = 0;
    int repeats = 5;
    int status = course::read_options(argc, argv, N, &task, &repeats);
    if (status != 0) return status == 1 ? 0 : 1;
    if (N == 0)
    {
        N = task == 3 ? 256 : 2048;
    }

    size_t result_count = N * N;
    if (task == 0)
    {
        result_count = 1; // Сумма всех элементов — одно число.
    }
    if (task == 2)
    {
        result_count = N; // Результат A * x — вектор, а не матрица.
    }

    auto A = std::make_unique<double[]>(N * N);
    auto B = std::make_unique<double[]>(N * N);
    auto x = std::make_unique<double[]>(N);
    auto serial = std::make_unique<double[]>(result_count);
    auto parallel = std::make_unique<double[]>(result_count);
    auto extra = std::make_unique<double[]>(result_count);

    for (size_t i = 0; i < N; ++i)
    {
        x[i] = double(i % 3) - 1.0;
        for (size_t j = 0; j < N; ++j)
        {
            A[i * N + j] = double((i + j) % 10) + 1.0;
            B[i * N + j] = double((2 * i + j) % 7) - 3.0;
        }
    }

    auto t1 = omp_get_wtime();
    for (int repeat = 0; repeat < repeats; ++repeat)
    {
        if (task == 0)
            serial[0] = matrix_sum(A.get(), N);
        else if (task == 1)
            matrix_add(A.get(), B.get(), serial.get(), N);
        else if (task == 2)
            matrix_vector(A.get(), x.get(), serial.get(), N);
        else
            matrix_multiply(A.get(), B.get(), serial.get(), N);
    }
    auto t2 = omp_get_wtime();

    for (int repeat = 0; repeat < repeats; ++repeat)
    {
        if (task == 0)
            parallel[0] = matrix_sum_parallel(A.get(), N);
        else if (task == 1)
            matrix_add_parallel(A.get(), B.get(), parallel.get(), N);
        else if (task == 2)
            matrix_vector_parallel(A.get(), x.get(), parallel.get(), N);
        else
            matrix_multiply_parallel(A.get(), B.get(), parallel.get(), N);
    }
    auto t3 = omp_get_wtime();

    double serial_ms = (t2 - t1) * 1000 / repeats;
    double parallel_ms = (t3 - t2) * 1000 / repeats;

    printf("Task: %d (0: sum A, 1: A+B, 2: A*x, 3: A*B), N: %zu\n", task, N);
    printf("Result [0]: %.0f\n", serial[0]);
    printf("Max error (parallel): %.6e\n", max_error(serial.get(), parallel.get(), result_count));
    printf("Time (serial): %.3f ms.\n", serial_ms);
    printf("Time (parallel): %.3f ms.\n", parallel_ms);
    if (parallel_ms > 0.0)
        printf("Speedup: %.2f x\n", serial_ms / parallel_ms);
    else
        printf("Speedup: timer resolution is too low; increase repeats.\n");

    // Сравнение порядка обхода памяти — продолжение второй практики.
    if (task == 2 || task == 3)
    {
        auto t4 = omp_get_wtime();
        for (int repeat = 0; repeat < repeats; ++repeat)
        {
            if (task == 2)
                matrix_vector_columns(A.get(), x.get(), extra.get(), N);
            else
                matrix_multiply_local(A.get(), B.get(), extra.get(), N);
        }
        auto t5 = omp_get_wtime();

        if (task == 2)
            printf("Time (serial columns): %.3f ms.\n", (t5 - t4) * 1000 / repeats);
        else
            printf("Time (parallel i-k-j): %.3f ms.\n", (t5 - t4) * 1000 / repeats);
        printf("Max error (extra): %.6e\n", max_error(serial.get(), extra.get(), result_count));
    }
    return 0;
}
