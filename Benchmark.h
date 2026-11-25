#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <iostream>
#include <iomanip>
#include <chrono>
#include <memory>
#include <utility>
#include "ShrdPtr.h"
#include "UniqPtr.h"

class Timer
{
private:
    std::chrono::high_resolution_clock::time_point start_time;
public:
    Timer() : start_time(std::chrono::high_resolution_clock::now()) {}

    double elapsed_ms() const
    {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> diff = end_time - start_time;
        return diff.count();
    }
};

// Вспомогательная функция для вывода строки таблицы
inline void print_row(size_t iterations, double time_raw, double time_my, double time_stl)
{
    double ratio = (time_stl > 0.0001) ? (time_my / time_stl) : 1.0;

    std::cout << "| " << std::setw(10) << iterations
              << " | " << std::setw(12) << std::fixed << std::setprecision(3) << time_raw
              << " | " << std::setw(12) << time_my
              << " | " << std::setw(12) << time_stl
              << " | " << std::setw(8) << std::setprecision(2) << ratio << "x |\n";
}

// 1. UniqPtr: Создание, доступ и удаление (одиночный объект)
inline void run_unique_create_benchmark(size_t iterations)
{
    volatile int sink = 0;

    Timer t_raw;
    for (size_t i = 0; i < iterations; ++i)
    {
        int* p = new int(static_cast<int>(i));
        sink += *p;
        delete p;
    }
    double time_raw = t_raw.elapsed_ms();

    Timer t_my;
    for (size_t i = 0; i < iterations; ++i)
    {
        UniqPtr<int> p(new int(static_cast<int>(i)));
        sink += *p;
    }
    double time_my = t_my.elapsed_ms();

    Timer t_stl;
    for (size_t i = 0; i < iterations; ++i)
    {
        std::unique_ptr<int> p(new int(static_cast<int>(i)));
        sink += *p;
    }
    double time_stl = t_stl.elapsed_ms();

    (void)sink;
    print_row(iterations, time_raw, time_my, time_stl);
}

// 2. UniqPtr: Чистое перемещение (без выделения памяти в цикле)
inline void run_unique_move_benchmark(size_t iterations)
{
    volatile int sink = 0;

    int* raw1 = new int(42);
    Timer t_raw;
    for (size_t i = 0; i < iterations; ++i)
    {
        int* raw2 = raw1;
        raw1 = nullptr;
        sink += *raw2;
        raw1 = raw2;
    }
    double time_raw = t_raw.elapsed_ms();
    delete raw1;

    UniqPtr<int> my1(new int(42));
    Timer t_my;
    for (size_t i = 0; i < iterations; ++i)
    {
        UniqPtr<int> my2 = std::move(my1);
        sink += *my2;
        my1 = std::move(my2);
    }
    double time_my = t_my.elapsed_ms();

    std::unique_ptr<int> stl1(new int(42));
    Timer t_stl;
    for (size_t i = 0; i < iterations; ++i)
    {
        std::unique_ptr<int> stl2 = std::move(stl1);
        sink += *stl2;
        stl1 = std::move(stl2);
    }
    double time_stl = t_stl.elapsed_ms();

    (void)sink;
    print_row(iterations, time_raw, time_my, time_stl);
}

// 3. UniqPtr<T[]>: Создание, индексация и удаление массивов (по 16 int)
inline void run_unique_array_benchmark(size_t iterations)
{
    volatile int sink = 0;
    constexpr int ARR_SIZE = 16;

    Timer t_raw;
    for (size_t i = 0; i < iterations; ++i)
    {
        int* arr = new int[ARR_SIZE];
        arr[0] = static_cast<int>(i);
        sink += arr[0];
        delete[] arr;
    }
    double time_raw = t_raw.elapsed_ms();

    Timer t_my;
    for (size_t i = 0; i < iterations; ++i)
    {
        UniqPtr<int[]> arr(new int[ARR_SIZE], ARR_SIZE);
        arr[0] = static_cast<int>(i);
        sink += arr[0];
    }
    double time_my = t_my.elapsed_ms();

    Timer t_stl;
    for (size_t i = 0; i < iterations; ++i)
    {
        std::unique_ptr<int[]> arr(new int[ARR_SIZE]);
        arr[0] = static_cast<int>(i);
        sink += arr[0];
    }
    double time_stl = t_stl.elapsed_ms();

    (void)sink;
    print_row(iterations, time_raw, time_my, time_stl);
}

// 4. ShrdPtr: Создание, копирование и удаление (полный цикл с ControlBlock)
inline void run_shared_create_benchmark(size_t iterations)
{
    volatile int sink = 0;

    Timer t_raw;
    for (size_t i = 0; i < iterations; ++i)
    {
        int* p1 = new int(static_cast<int>(i));
        int* p2 = p1;
        sink += *p2;
        delete p1;
    }
    double time_raw = t_raw.elapsed_ms();

    Timer t_my;
    for (size_t i = 0; i < iterations; ++i)
    {
        ShrdPtr<int> p1(new int(static_cast<int>(i)));
        ShrdPtr<int> p2 = p1;
        sink += *p2;
    }
    double time_my = t_my.elapsed_ms();

    Timer t_stl;
    for (size_t i = 0; i < iterations; ++i)
    {
        std::shared_ptr<int> p1(new int(static_cast<int>(i)));
        std::shared_ptr<int> p2 = p1;
        sink += *p2;
    }
    double time_stl = t_stl.elapsed_ms();

    (void)sink;
    print_row(iterations, time_raw, time_my, time_stl);
}

// 5. ShrdPtr: Чистое копирование (стресс-тест атомарного ControlBlock без new/delete)
inline void run_shared_copy_benchmark(size_t iterations)
{
    volatile int sink = 0;

    int* raw1 = new int(42);
    Timer t_raw;
    for (size_t i = 0; i < iterations; ++i)
    {
        int* raw2 = raw1;
        sink += *raw2;
    }
    double time_raw = t_raw.elapsed_ms();
    delete raw1;

    ShrdPtr<int> my1(new int(42));
    Timer t_my;
    for (size_t i = 0; i < iterations; ++i)
    {
        ShrdPtr<int> my2 = my1;
        sink += *my2;
    }
    double time_my = t_my.elapsed_ms();

    std::shared_ptr<int> stl1(new int(42));
    Timer t_stl;
    for (size_t i = 0; i < iterations; ++i)
    {
        std::shared_ptr<int> stl2 = stl1;
        sink += *stl2;
    }
    double time_stl = t_stl.elapsed_ms();

    (void)sink;
    print_row(iterations, time_raw, time_my, time_stl);
}

#endif // BENCHMARK_H