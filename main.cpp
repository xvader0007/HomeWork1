#include <iostream>
#include <string>
#include "Benchmark.h"

void print_header(const std::string& title)
{
    std::cout << "\n=== " << title << " (time in ms) ===\n";
    std::cout << "+------------+--------------+--------------+--------------+-----------+\n";
    std::cout << "| Iterations |  Raw Pointer |  My SmartPtr |  STL SmartPtr|  My / STL |\n";
    std::cout << "+------------+--------------+--------------+--------------+-----------+\n";
}

void print_footer()
{
    std::cout << "+------------+--------------+--------------+--------------+-----------+\n";
}

int main()
{
    const size_t alloc_sizes[] = {100, 10000, 100000, 1000000, 5000000};
    const size_t fast_sizes[]  = {10000, 100000, 1000000, 10000000};

    print_header("1. UNIQUE: Create / Access / Destroy");
    for (size_t n : alloc_sizes) run_unique_create_benchmark(n);
    print_footer();

    print_header("2. UNIQUE: Pure Move Semantics (no heap alloc)");
    for (size_t n : fast_sizes) run_unique_move_benchmark(n);
    print_footer();

    print_header("3. UNIQUE ARRAY: Create / Index / Destroy (16 ints)");
    for (size_t n : alloc_sizes) run_unique_array_benchmark(n);
    print_footer();

    print_header("4. SHARED: Create / Copy / Destroy (2 allocs)");
    for (size_t n : alloc_sizes) run_shared_create_benchmark(n);
    print_footer();

    print_header("5. SHARED: Pure Copy / Ref-Counting (ControlBlock stress)");
    for (size_t n : fast_sizes) run_shared_copy_benchmark(n);
    print_footer();

    return 0;
}