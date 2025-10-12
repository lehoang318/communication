#include "P2P_Endpoint.hpp"

namespace comm {

inline void P2P_Endpoint::start() {
    mExitFlag = false;

    mpRxThread.reset(new std::thread(&P2P_Endpoint::runRx, this));
    mpRxThread->detach();

    mpTxThread.reset(new std::thread(&P2P_Endpoint::runTx, this));
    mpTxThread->detach();
}

inline void P2P_Endpoint::stop() {
    mExitFlag = true;

    for (int i = 0; (RETRY_LIMIT > i) && isAlive(); i++) {
        sleep_for(RETRY_BREAK_US);
    }

    if (isAlive()) {
        LOGE("Failed to terminate internal threads!!!\n");
    }
}

inline bool P2P_Endpoint::isAlive() {
    return (mRxAliveFlag || mTxAliveFlag);
}

inline bool P2P_Endpoint::send(std::unique_ptr<Packet>& pPacket) {
    if (pPacket) {
        if (mTxQueue.enqueue(pPacket)) {
            return true;
        } else {
            LOGE("Tx Queue is full!!!\n");
        }
    } else {
        LOGE("Tx packet must not be empty!!!\n");
    }

    return false;
}

inline bool P2P_Endpoint::send(std::unique_ptr<Packet>&& pPacket) {
    if (pPacket) {
        if (mTxQueue.enqueue(pPacket)) {
            return true;
        } else {
            LOGE("Tx Queue is full!!!\n");
        }
    } else {
        LOGE("Tx packet must not be empty!!!\n");
    }

    return false;
}

inline bool P2P_Endpoint::recvAll(std::deque<std::unique_ptr<Packet>>& pRxPackets, const bool wait) {
    return mDecoder.dequeue(pRxPackets, wait);
}

}  // namespace comm
