#ifndef __P2P_ENPOINT_HPP__
#define __P2P_ENPOINT_HPP__

#include "Encoder.hpp"
#include "Packet.hpp"
#include "SyncQueue.hpp"

#include <atomic>
#include <cstdint>
#include <deque>
#include <errno.h>
#include <memory>
#include <mutex>
#include <unistd.h>

#ifdef __WIN32__
#include <WinDef.h>
#endif  // __WIN32__

namespace comm {

#ifdef __WIN32__
static constexpr DWORD RX_TIMEOUT_S = 1;
#else   // __WIN32__
static constexpr time_t RX_TIMEOUT_S = 1LL;
#endif  // __WIN32__
static constexpr int TX_RETRY_COUNT = 3;

class P2P_Endpoint {
   public:
    virtual ~P2P_Endpoint() {}

    /**
     * @brief Put package(s) into Tx queue (non-blocking).
     */
    bool send(std::unique_ptr<Packet>& pPacket);
    bool send(std::unique_ptr<Packet>&& pPacket);

    /**
     * @brief Get data from Rx queue.
     */
    bool recvAll(std::deque<std::unique_ptr<Packet>>& pRxPackets, bool wait = true);

    virtual bool isPeerConnected();
    virtual void close() = 0;

   protected:
    P2P_Endpoint() {
        mpRxBuffer.reset(new uint8_t[MAX_FRAME_SIZE]);
        mTransactionId = 0;
    }

    /**
     * @brief Process received data from Peer (non-blocking).
     */
    bool proceedRx();

    /**
     * @brief Process transmit requests from higher layers (non-blocking).
     */
    bool proceedTx(bool discard = false);

    /**
     * @brief Read Rx buffer (non-blocking).
     *
     * @param[in] pBuffer Pointer to the buffer to store the data.
     * @param[in] limit The maximum number of bytes to read.
     * @return The number of bytes read from Rx buffer, or -1 if an error occurs.
     */
    virtual ssize_t lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) = 0;

    /**
     * @brief Write data to Tx buffer (non-blocking).
     *
     * @param[in] pData Pointer to the data to write.
     * @param[in] size The size of the data to write.
     * @return The number of bytes written, or -1 if an error occurs.
     */
    virtual ssize_t lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) = 0;

   private:
    std::unique_ptr<uint8_t[]> mpRxBuffer;
    Decoder mDecoder;
    std::mutex mRxMutex;

    dstruct::SyncQueue<Packet> mTxQueue;
    uint16_t mTransactionId;
    std::mutex mTxMutex;
};  // class P2P_Endpoint

}  // namespace comm

#include "inline/P2P_Endpoint.inl"

#endif  // __P2P_ENPOINT_HPP__
