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
#include <thread>
#include <unistd.h>

#ifdef __WIN32__
#include <WinDef.h>
#else   // __WIN32__
#include <sys/types.h>
#endif  // __WIN32__

namespace comm {

#ifdef __WIN32__
static constexpr DWORD RX_TIMEOUT_S = 1;
#else   // __WIN32__
static constexpr time_t RX_TIMEOUT_S = 1LL;
#endif  // __WIN32__
static constexpr int TX_RETRY_COUNT = 3;
static constexpr long TX_RETRY_BREAK_US = 100L;

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

   protected:
    P2P_Endpoint() {
        mTransactionId = 0;
    }

    /**
     * @brief Start internal threads. The method must only be called by the constructor of a derived class.
     */
    void start();

    /**
     * @brief Stop internal threads. The method must only be called by the destructor of a derived class.
     */
    void stop();

    /**
     * @brief Rx thread.
     */
    virtual void runRx();

    /**
     * @brief Tx thread.
     */
    virtual void runTx();

    /**
     * @brief Return true if Rx Pipe is operational.
     */
    virtual bool checkRxPipe() { return true; }

    /**
     * @brief Return true if Tx Pipe is operational.
     */
    virtual bool checkTxPipe() { return true; }

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

    std::unique_ptr<std::thread> mpRxThread;
    std::unique_ptr<std::thread> mpTxThread;
    std::atomic<bool> mExitFlag;

   private:
    Decoder mDecoder;

    dstruct::SyncQueue<Packet> mTxQueue;
    uint16_t mTransactionId;
};  // class P2P_Endpoint

}  // namespace comm

#include "inline/P2P_Endpoint.inl"

#endif  // __P2P_ENPOINT_HPP__
