#pragma once
#include <chrono>
#include <iostream>

namespace sqbinding {
namespace detail {
struct time_guard {
    time_guard() {
        start = std::chrono::steady_clock::now();
    }
    ~time_guard() {
        std::cout
            << "Time difference = "
            << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()
            << "[ms]\n";
    }
    std::chrono::steady_clock::time_point start;
};
} // namespace detail
} // namespace sqbinding
