#include "P2P_Endpoint.hpp"

namespace comm {

void P2P_Endpoint::runRx() {
    mRxAliveFlag = true;
    static std::unique_ptr<uint8_t[]> mpRxBuffer(new uint8_t[MAX_FRAME_SIZE]);

    while (!mExitFlag) {
        if (!checkRxPipe()) {
            LOGI("Rx Pipe was broken!\n");
            break;
        }

        ssize_t byteCount = lread(mpRxBuffer, MAX_FRAME_SIZE);
        if (0 > byteCount) {
            LOGI("Could not read from lower layer!\n");
            break;
        } else if (0 < byteCount) {
            mDecoder.feed(mpRxBuffer, byteCount);
        } else {
            // Do nothing
        }
    }

    mRxAliveFlag = false;
}

void P2P_Endpoint::runTx() {
    mTxAliveFlag = true;

    std::unique_ptr<uint8_t[]> pEncodedData;
    size_t encodedSize;
    ssize_t byteCount = 0;

    while (!mExitFlag) {
        if (!checkTxPipe()) {
            LOGI("Tx Pipe was broken!\n");
            break;
        }

        std::deque<std::unique_ptr<Packet>> pTxPackets;
        if (!mTxQueue.dequeue(pTxPackets) || (0 >= pTxPackets.size())) {
            // Tx queue is empty!
            continue;
        }

        LOGD("%zu packets in Tx queue\n", pTxPackets.size());

        for (auto& pPacket : pTxPackets) {
            encode(
                pPacket->getPayload(), pPacket->getPayloadSize(), mTransactionId++,
                pEncodedData, encodedSize);

            if ((!pEncodedData) || (0 == encodedSize)) {
                LOGI("Could not encode data!\n");
                continue;
            }

            byteCount = lwrite(pEncodedData, encodedSize);
            if (0 > byteCount) {
                LOGI("Could not write to lower layer!\n");
                break;
            } else {
                LOGD("Wrote %zd bytes\n", byteCount);  // [TODO] byteCount < encodedSize
            }
        }

        if (0 > byteCount) {
            break;
        }
    }

    mTxAliveFlag = false;
}

}  // namespace comm
