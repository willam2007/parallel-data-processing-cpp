#define main practice01_entry
#include "../main.cpp"
#undef main
#define main practice02_entry
#include "../practice02.cpp"
#undef main
#define main practice03_entry
#include "../practice03.cpp"
#undef main
#include <vector>
#include <cstdlib>

int checks = 0;

void check(const char* name, const double* actual, const double* expected, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        if (!std::isfinite(actual[i]) || actual[i] != expected[i])
        {
            std::printf("FAIL %s, index %zu: %.17g != %.17g\n", name, i, actual[i], expected[i]);
            std::exit(1);
        }
    }
    ++checks;
}

using Kernel = void (*)(const double*, const double*, double*, size_t);

int main()
{
    omp_set_dynamic(0);
    const int thread_counts[] = {1, 2, 3, 5, 8, 11};
    const size_t lengths[] = {0, 1, 2, 3, 5, 7, 17};
    const Kernel additions[] = {matrix_add, matrix_add_parallel};
    const Kernel vectors[] = {matrix_vector, matrix_vector_parallel, matrix_vector_columns};
    const Kernel products[] = {matrix_multiply, matrix_multiply_parallel, matrix_multiply_local};

    // Ответы вычислены вручную и независимо от функций из учебного файла.
    const double handA[] = {1, 2, 3, 4};
    const double handB[] = {5, 6, 7, 8};
    const double handX[] = {10, 20};
    const double handAdd[] = {6, 8, 10, 12};
    const double handVector[] = {50, 110};
    const double handProduct[] = {19, 22, 43, 50};
    const double handSum = 10;

    for (int threads : thread_counts)
    {
        omp_set_num_threads(threads);
        double output[4];
        double s = matrix_sum(handA, 2);
        check("hand sum", &s, &handSum, 1);
        s = matrix_sum_parallel(handA, 2);
        check("hand sum parallel", &s, &handSum, 1);
        for (Kernel fn : additions)
        {
            fn(handA, handB, output, 2);
            check("hand add", output, handAdd, 4);
        }
        for (Kernel fn : vectors)
        {
            fn(handA, handX, output, 2);
            check("hand vector", output, handVector, 2);
        }
        for (Kernel fn : products)
        {
            fn(handA, handB, output, 2);
            check("hand product", output, handProduct, 4);
        }

        const size_t array_lengths[] = {0, 1, 3, 7, 17, 1000};
        double (*sum_methods[])(const double*, size_t) = {
            sum, sum_reverse, sum_parallel, sum_par, sum_loc, sum_blocks
        };
        for (size_t n : array_lengths)
        {
            std::vector<double> data(n);
            double expected = 0.0;
            for (size_t i = 0; i < n; ++i)
            {
                data[i] = double(int(i % 11) - 5) * 0.5;
                expected += data[i];
            }
            for (int repeat = 0; repeat < 2; ++repeat)
            {
                for (auto method : sum_methods)
                {
                    double actual = method(data.data(), n);
                    check("array sum", &actual, &expected, 1);
                }
            }
        }
        for (size_t n : lengths)
        {
            std::vector<double> A(n * n), B(n * n), x(n);
            std::vector<double> add(n * n), product(n * n, 0.0), y(n, 0.0);
            std::vector<double> result(n * n, -777.0);
            double sum_expected = 0;
            for (size_t p = 0; p < n * n; ++p)
            {
                A[p] = double(int(p % 7) - 3) * 0.5;
                B[p] = double(int((p * 3) % 11) - 5) * 0.25;
                add[p] = A[p] + B[p];
                sum_expected += A[p];
            }
            for (size_t j = 0; j < n; ++j)
                x[j] = double(int(j % 5) - 2) * 0.5;
            // Другой способ индексации для независимого эталона A*x.
            for (size_t p = 0; p < n * n; ++p)
                y[p / n] += A[p] * x[p % n];
            // Другой порядок циклов для независимого эталона A*B.
            for (size_t k = 0; k < n; ++k)
                for (size_t i = 0; i < n; ++i)
                    for (size_t j = 0; j < n; ++j)
                        product[i * n + j] += A[i * n + k] * B[k * n + j];

            for (int repeat = 0; repeat < 2; ++repeat)
            {
                s = matrix_sum(A.data(), n);
                check("sum", &s, &sum_expected, 1);
                s = matrix_sum_parallel(A.data(), n);
                check("sum parallel", &s, &sum_expected, 1);
                for (Kernel fn : additions)
                {
                    fn(A.data(), B.data(), result.data(), n);
                    check("add", result.data(), add.data(), n * n);
                }
                for (Kernel fn : vectors)
                {
                    fn(A.data(), x.data(), result.data(), n);
                    check("vector", result.data(), y.data(), n);
                }
                for (Kernel fn : products)
                {
                    fn(A.data(), B.data(), result.data(), n);
                    check("product", result.data(), product.data(), n * n);
                }
            }
        }
    }
    const double ref[] = {1, 2, 3};
    const double changed[] = {1, -2, 4};
    double error = max_error(ref, changed, 3);
    const double expected_error = 4;
    check("max_error", &error, &expected_error, 1);
    std::printf("PASS: %d checks; hand-computed cases, array/matrix algorithms; empty inputs, fractions, repeats, 1/2/3/5/8/11 threads.\n", checks);
}

