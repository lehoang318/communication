#ifndef __ENCODER_HPP__
#define __ENCODER_HPP__

#include "Packet.hpp"
#include "SyncQueue.hpp"
#include "common.hpp"

#include <cstdint>
#include <cstring>
#include <deque>
#include <memory>

namespace comm {

/**
 * @brief Encodes the given data into a packet.
 *
 * @param[in] pData Pointer to the data to encode.
 * @param[in] size Size in bytes of the data to encode.
 * @param[in] tid Transaction ID of the packet.
 * @param[out] pEncodedData Pointer to the buffer to store the encoded data.
 * @param[out] encodedSize Size of the encoded data.
 *
 * @return True if the encoding is successful, false otherwise.
 */
bool encode(
    const std::unique_ptr<uint8_t[]>& pData, const size_t& size, const uint16_t& tid,
    std::unique_ptr<uint8_t[]>& pEncodedData, size_t& encodedSize);

enum DECODING_STATES {
    E_SF,
    E_TID,
    E_SIZE,
    E_PAYLOAD,
    E_VALIDATION
};

class Decoder {
   public:
    Decoder() : mState(E_SF), mCachedTransactionId(-1) {}
    virtual ~Decoder() { resetBuffer(); }

    /**
     * @brief Feeds data to the decoder.
     *
     * @param[in] pdata Pointer to the data to feed.
     * @param[in] size Size of the data to feed.
     */
    void feed(const std::unique_ptr<uint8_t[]>& pdata, const size_t& size);

    /**
     * @brief Dequeues a list of packets from the decoder.
     *
     * @param[out] pPackets The dequeued packets.
     * @param[in] wait Whether to wait until at least one packet is available.
     * @return True if at least a packet is successfully dequeued, false otherwise.
     */
    bool dequeue(std::deque<std::unique_ptr<Packet>>& pPackets, bool wait = true);

   private:
    /**
     * @brief Proceeds the next byte from input data.
     */
    void proceed(const uint8_t& b);

    /**
     * @brief Resets the decoder's internal buffer.
     */
    void resetBuffer();

    DECODING_STATES mState;

    size_t mPayloadSize;
    std::unique_ptr<uint8_t[]> mpPayload;

    /**
     * @brief Decoded packets shall be pushed to this queue.
     */
    dstruct::SyncQueue<Packet> mDecodedQueue;
    int mTransactionId;
    int mCachedTransactionId;
};  // class Decoder

}  // namespace comm

#include "inline/Encoder.inl"

#endif  // __ENCODER_HPP__
