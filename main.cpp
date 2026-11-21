#include <iostream>
#include "Benchmark.h"

void print_header(const std::string& title) {
    std::cout << "\n=== " << title << " (time in ms) ===\n";
    std::cout << "+--------------+----------------+----------------+----------------+\n";
    std::cout << "|   Objects    |   Raw Pointer  |   My SmartPtr  |   STL SmartPtr |\n";
    std::cout << "+--------------+----------------+----------------+----------------+\n";
}

void print_footer() {
    std::cout << "+--------------+----------------+----------------+----------------+\n";
}

int main() {
    const size_t test_sizes[] = {100, 10000, 100000, 1000000, 10000000};

    //Сравнение Unique-указателей
    print_header("UNIQUE POINTERS (UniqPtr vs std::unique_ptr)");
    for (size_t n : test_sizes) {
        run_unique_benchmark(n);
    }
    print_footer();

    //Сравнение Shared-указателей
    print_header("SHARED POINTERS (ShrdPtr vs std::shared_ptr)");
    for (size_t n : test_sizes) {
        run_shared_benchmark(n);
    }
    print_footer();

    return 0;
}