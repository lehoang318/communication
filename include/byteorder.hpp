#ifndef __BYTEORDER_HPP__
#define __BYTEORDER_HPP__

enum BYTE_ORDER {
    ELITTLE_ENDIAN,
    EBIG_ENDIAN
};

/**
 * @brief Get the byte order of the current platform.
 *
 * This method returns the byte order of the current platform. The byte order
 * determines the order in which bytes are stored in memory. The most common
 * byte orders are Big Endian and Little Endian.
 *
 * @return The byte order of the current platform.
 * @retval EBIG_ENDIAN if the byte order is Big Endian.
 * @retval ELITTLE_ENDIAN if the byte order is Little Endian.
 *
 * @see https://stackoverflow.com/questions/1001307/detecting-endianness-programmatically-in-a-c-program
 */
inline BYTE_ORDER get_byte_order() {
    const int value = 1;
    const void *address = static_cast<const void *>(&value);
    const unsigned char *least_significant_address = static_cast<const unsigned char *>(address);
    if (*least_significant_address == 0x01) {
        return ELITTLE_ENDIAN;
    } else {
        return EBIG_ENDIAN;
    }
}

#endif  // __BYTEORDER_HPP__
