#pragma once

// Параметры запуска отделены от учебных алгоритмов.
#include <charconv>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <limits>
#include <system_error>

namespace course
{
// 0 — продолжить, 1 — показана справка, 2 — неверные параметры.
inline int read_options(int argc, char** argv, size_t& size,
                        int* task = nullptr, int* repeats = nullptr)
{
    for (int i = 1; i < argc; ++i)
    {
        const char* flag = argv[i];
        if (std::strcmp(flag, "--help") == 0)
        {
            std::printf("Options: --size N");
            if (task) std::printf(" --task 0|1|2|3 --repeats N");
            std::printf("\nN must be positive. Matrix --size is the side length.\n");
            return 1;
        }
        bool is_size = std::strcmp(flag, "--size") == 0;
        bool is_task = task && std::strcmp(flag, "--task") == 0;
        bool is_repeats = repeats && std::strcmp(flag, "--repeats") == 0;
        if ((!is_size && !is_task && !is_repeats) || i + 1 >= argc)
        {
            std::fprintf(stderr, "Unknown option or missing value: %s. Use --help.\n", flag);
            return 2;
        }
        const char* text = argv[++i];
        unsigned long long value = 0;
        auto parsed = std::from_chars(text, text + std::strlen(text), value);
        if (parsed.ec != std::errc() || parsed.ptr != text + std::strlen(text))
        {
            std::fprintf(stderr, "Invalid integer for %s: %s\n", flag, text);
            return 2;
        }
        if (is_size)
        {
            if (value == 0 || value > std::numeric_limits<size_t>::max() / sizeof(double))
            {
                std::fprintf(stderr, "--size must be positive and fit in memory addressing.\n");
                return 2;
            }
            size = size_t(value);
        }
        else if (is_task)
        {
            if (value > 3)
            {
                std::fprintf(stderr, "--task must be 0, 1, 2 or 3.\n");
                return 2;
            }
            *task = int(value);
        }
        else
        {
            if (value == 0 || value > unsigned(std::numeric_limits<int>::max()))
            {
                std::fprintf(stderr, "--repeats must be a positive int.\n");
                return 2;
            }
            *repeats = int(value);
        }
    }
    if (task && size != 0 && size > (std::numeric_limits<size_t>::max() / sizeof(double)) / size)
    {
        std::fprintf(stderr, "N*N*sizeof(double) overflows for this --size.\n");
        return 2;
    }
    return 0;
}
}
