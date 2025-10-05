#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <chrono>
#include <cstdint>
#include <cstdio>

/**
 * @brief Logging wrapper functions.
 */
#define LOGI(...) printf(__VA_ARGS__)
#define LOGE(...) fprintf(stderr, __VA_ARGS__)

#ifdef DEBUG
#define LOGD(...) printf(__VA_ARGS__)
#else  // DEBUG
#define LOGD(...)
#endif

/**
 * @brief Returns the current time point in the monotonic clock.
 */
inline std::chrono::time_point<std::chrono::steady_clock> get_monotonic_clock() {
    return std::chrono::steady_clock::now();
}

/**
 * @brief Returns the elapsed time from the 1st call in microseconds.
 */
inline int64_t get_elapsed_realtime_us() {
    static auto base = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - base).count();
}

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
constexpr size_t MAX_PAYLOAD_SIZE = 1024UL;  // Avoid network fragmentation

constexpr size_t MAX_FRAME_SIZE = SF_SIZE + SIZE_OF_TID + SIZE_OF_PAYLOAD_SIZE + MAX_PAYLOAD_SIZE + EF_SIZE;

/**
 * @brief Returns `false` if the payload size is greater than `MAX_PAYLOAD_SIZE`.
 */
inline bool validate_payload_size(const size_t& payload_size) {
    return (MAX_PAYLOAD_SIZE >= payload_size);
}

}  // namespace comm

#endif  // _COMMON_HPP_
