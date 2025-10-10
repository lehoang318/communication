#include "common.hpp"

#include <ctime>
#include <cerrno>

static auto tp0 = std::chrono::steady_clock::now();

int64_t get_elapsed_realtime_us() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - tp0).count();
}

int64_t get_elapsed_realtime_us(monotonic_time_point tp) {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - tp).count();
}

void sleep_for(long duration_us) {
    struct timespec target_time;
    if (clock_gettime(CLOCK_MONOTONIC, &target_time) == -1) {
        perror("clock_gettime failed");
        return;
    }

    if (duration_us < 0 || duration_us >= 1000000000L) {
        LOGE("`duration_us` must be between 0 and 999,999,999!\n");
        duration_us = 0;
    }

    long added_sec = duration_us / US_PER_S;
    long added_nsec = (duration_us % US_PER_S) * NS_PER_US;

    target_time.tv_sec += added_sec;
    target_time.tv_nsec += added_nsec;

    // 4. Normalize the timespec structure (handle nanosecond overflow)
    while (target_time.tv_nsec >= NS_PER_S) {
        target_time.tv_nsec -= NS_PER_S;
        target_time.tv_sec++;
    }

    int ret = EINTR;
    int i = 0;
    do {
        ret = clock_nanosleep(
            CLOCK_MONOTONIC,
            TIMER_ABSTIME,      // Sleep until the absolute time specified by deadline
            &target_time,       // The absolute target time
            NULL                // Remaining time is unused with TIMER_ABSTIME
        );

        i++;
    } while ((3 > i) && (ret == EINTR));

    if (0 != ret) {
        LOGE("Encounterred error when executing `clock_nanosleep()`: %d!\n", errno);
    }
}
