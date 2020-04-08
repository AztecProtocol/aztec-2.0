#include "./profiler.hpp"

#ifndef NO_MULTITHREADING
#include "omp.h"
#endif

#if defined(BARRETENBERG_PROFILING)
#include <array>
#include <chrono>
#include <iostream>
#endif

namespace profiler {

#if defined(BARRETENBERG_PROFILING)
namespace {
std::array<std::chrono::steady_clock::time_point, 64> thread_timings;
}

void start_measurement()
{
#ifndef NO_MULTITHREADING
    const size_t thread_id = static_cast<size_t>(omp_get_thread_num());
#else
    const size_t thread_id = 1UL;
#endif
    thread_timings[thread_id] = std::chrono::steady_clock::now();
}

void end_measurement(const std::string& message)
{
#ifndef NO_MULTITHREADING
    const size_t thread_id = static_cast<size_t>(omp_get_thread_num());
#else
    const size_t thread_id = 1UL;
#endif
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::chrono::milliseconds diff =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - thread_timings[thread_id]);
    std::cout << message << " execution time: " << diff.count() << "ms" << std::endl;
}
#else
void start_measurement() {}
void end_measurement(const std::string&) {}
#endif

} // namespace profiler
