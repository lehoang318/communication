#include "common.hpp"

#include <cerrno>
#include <ctime>
#include <thread>

static auto tp0 = std::chrono::steady_clock::now();

int64_t get_elapsed_realtime_us() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - tp0).count();
}

int64_t get_elapsed_realtime_us(const monotonic_time_point& tp) {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - tp).count();
}

void sleep_for(uint32_t duration_us) {
    auto t_wakeup = monotonic_now() + std::chrono::microseconds(duration_us);
    std::this_thread::sleep_until(t_wakeup);
}
