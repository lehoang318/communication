#include "P2P_Endpoint.hpp"

namespace comm {

inline void P2P_Endpoint::start() {
    mExitFlag = false;
    mpRxThread.reset(new std::thread(&P2P_Endpoint::runRx, this));
    mpTxThread.reset(new std::thread(&P2P_Endpoint::runTx, this));
}

inline void P2P_Endpoint::stop() {
    mExitFlag = true;
    if ((mpRxThread) && (mpRxThread->joinable())) {
        mpRxThread->join();
    }

    if ((mpTxThread) && (mpTxThread->joinable())) {
        mpTxThread->join();
    }
}

inline bool P2P_Endpoint::send(std::unique_ptr<Packet>& pPacket) {
    if (pPacket) {
        if (!mTxQueue.enqueue(pPacket)) {
            LOGE("Tx Queue is full!\n");
        }
        return true;
    } else {
        LOGI("Tx packet must not be empty!\n", __func__, __LINE__);
        return false;
    }
}

inline bool P2P_Endpoint::send(std::unique_ptr<Packet>&& pPacket) {
    if (pPacket) {
        if (!mTxQueue.enqueue(pPacket)) {
            LOGE("Tx Queue is full!\n");
        }
        return true;
    } else {
        LOGI("Tx packet must not be empty!\n", __func__, __LINE__);
        return false;
    }
}

inline bool P2P_Endpoint::recvAll(std::deque<std::unique_ptr<Packet>>& pRxPackets, bool wait) {
    return mDecoder.dequeue(pRxPackets, wait);
}

inline bool P2P_Endpoint::isPeerConnected() {
    return true;
}

}  // namespace comm
