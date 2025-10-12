#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <chrono>
#include <cstdint>
#include <cstdio>

#define NS_PER_US (1000L)
#define NS_PER_MS (1000000L)
#define NS_PER_S (1000000000L)
#define US_PER_MS (1000L)
#define US_PER_S (1000000L)

/**
 * @brief Logging wrapper functions.
 */
#define LOGI(format, ...) printf("[I][%s:%d] " format, __func__, __LINE__, ##__VA_ARGS__)
#define LOGW(format, ...) printf("[W][%s:%d] " format, __func__, __LINE__, ##__VA_ARGS__)
#define LOGE(format, ...) fprintf(stderr, "[E][%s:%d] " format, __func__, __LINE__, ##__VA_ARGS__)

#ifdef DEBUG
#define LOGD(format, ...) printf("[D][%s:%d] " format, __func__, __LINE__, ##__VA_ARGS__)
#else  // DEBUG
#define LOGD(...)
#endif

typedef std::chrono::time_point<std::chrono::steady_clock> monotonic_time_point;

/**
 * @brief Returns the current time point in the Monotonic Clock.
 */
inline monotonic_time_point monotonic_now() {
    return std::chrono::steady_clock::now();
}

/**
 * @brief Returns the elapsed time since the process started (using Monotonic Clock).
 */
int64_t get_elapsed_realtime_us();

/**
 * @brief Returns the elapsed time since input time point (using Monotonic Clock).
 */
int64_t get_elapsed_realtime_us(const monotonic_time_point& tp);

/**
 * @brief Puts the current thread to sleep for a specified duration (us) using CLOCK_MONOTONIC.
 */
void sleep_for(uint32_t duration_us);

namespace comm {

// Frame Structure
// 0xF0 (uint8_t) | Transaction ID (uint16_t LE) | Size of Payload (uint32_t LE) | Payload | 0x0F (uint8_t)
constexpr uint8_t SF = 0xF0U;
constexpr size_t SF_SIZE = 1UL;

constexpr uint8_t EF = 0x0FU;
constexpr size_t EF_SIZE = 1UL;

constexpr size_t SIZE_OF_TID = 2UL;
constexpr int32_t MAX_VALUE_OF_TID = 0xFFFF;  // Must be less than ((1 << (SIZE_OF_TID << 3)) - 1)

constexpr size_t SIZE_OF_PAYLOAD_SIZE = 4UL;
constexpr size_t MAX_PAYLOAD_SIZE = 1024UL;  // Avoid IP Fragmentation

constexpr size_t MAX_FRAME_SIZE = SF_SIZE + SIZE_OF_TID + SIZE_OF_PAYLOAD_SIZE + MAX_PAYLOAD_SIZE + EF_SIZE;

/**
 * @brief Returns `false` if the payload size is greater than `MAX_PAYLOAD_SIZE`.
 */
inline bool validate_payload_size(const size_t& payload_size) {
    return (MAX_PAYLOAD_SIZE >= payload_size);
}

}  // namespace comm

#endif  // _COMMON_HPP_
