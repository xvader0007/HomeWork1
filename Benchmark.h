#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <iostream>
#include <iomanip>
#include <chrono>
#include <memory>
#include "SmartPtrs.h"

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

//Одиночное владение
inline void run_unique_benchmark(size_t iterations)
{
    volatile int sink = 0;

    //Сырые указатели
    Timer t_raw;
    for (size_t i = 0; i < iterations; ++i)
    {
        int* p = new int(static_cast<int>(i));
        sink += *p;
        delete p;
    }
    double time_raw = t_raw.elapsed_ms();

    //UniqPtr
    Timer t_my;
    for (size_t i = 0; i < iterations; ++i)
    {
        UniqPtr<int> p(new int(static_cast<int>(i)));
        sink += *p;
    }
    double time_my = t_my.elapsed_ms();

    //STL
    Timer t_stl;
    for (size_t i = 0; i < iterations; ++i)
    {
        std::unique_ptr<int> p(new int(static_cast<int>(i)));
        sink += *p;
    }
    double time_stl = t_stl.elapsed_ms();

    (void)sink;

    std::cout << "| " << std::setw(12) << iterations
              << " | " << std::setw(14) << std::fixed << std::setprecision(3) << time_raw
              << " | " << std::setw(14) << time_my
              << " | " << std::setw(14) << time_stl << " |\n";
}

//Разделяемое владение
inline void run_shared_benchmark(size_t iterations)
{
    volatile int sink = 0;

    //Сырые указатели
    Timer t_raw;
    for (size_t i = 0; i < iterations; ++i)
    {
        int* p1 = new int(static_cast<int>(i));
        int* p2 = p1;
        sink += *p2;
        delete p1;
    }
    double time_raw = t_raw.elapsed_ms();

    //ShrdPtr
    Timer t_my;
    for (size_t i = 0; i < iterations; ++i)
    {
        ShrdPtr<int> p1(new int(static_cast<int>(i)));
        ShrdPtr<int> p2 = p1;
        sink += *p2;
    }
    double time_my = t_my.elapsed_ms();

    //STL
    Timer t_stl;
    for (size_t i = 0; i < iterations; ++i)
    {
        std::shared_ptr<int> p1(new int(static_cast<int>(i)));
        std::shared_ptr<int> p2 = p1;
        sink += *p2;
    }
    double time_stl = t_stl.elapsed_ms();

    (void)sink;

    std::cout << "| " << std::setw(12) << iterations
              << " | " << std::setw(14) << std::fixed << std::setprecision(3) << time_raw
              << " | " << std::setw(14) << time_my
              << " | " << std::setw(14) << time_stl << " |\n";
}

#endif // BENCHMARK_H