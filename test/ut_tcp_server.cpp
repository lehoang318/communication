#include "Packet.hpp"
#include "TcpServer.hpp"
#include "common.hpp"
#include "test_vectors.hpp"
#include "util.hpp"

#include <cinttypes>
#include <cstring>
#include <deque>

#define EP_NAME "TcpServer"

int main(int argc, char** argv) {
    if (2 > argc) {
        LOGE("Usage: %s <Local Port>\n", argv[0]);
        return 1;
    }

    std::unique_ptr<comm::TcpServer> pTcpServer = comm::TcpServer::create(static_cast<uint16_t>(atoi(argv[1])));

    if (!pTcpServer) {
        LOGE("Could not create TCP Server which listens at port %s!\n", argv[1]);
        return 1;
    }

    LOGI("TCP Server is ready, waiting for connection requests ...\n");
    int errorCode = 0;
    std::unique_ptr<comm::P2P_Endpoint> pEndpoint = pTcpServer->waitForClient(errorCode, 5000);
    if (0 != errorCode) {
        LOGE("Encountered errors while waiting for connection requests: %d!!!\n", errorCode);
        return 1;
    } else if (!pEndpoint) {
        LOGD("Timeout!\n");
        return 1;
    } else {
        LOGI("Accepted one client, press enter to sent data ...\n");
    }

    getchar();

    for (size_t i = 0; i < vectors.size(); i++) {
        LOGI("[%" PRId64 " (us)] Sending packet %zu (%zu bytes) ...\n",
             get_elapsed_realtime_us(),
             i, vectors_sizes[i]);
#ifdef USE_RAW_POINTER
        pEndpoint->send(comm::Packet::create(vectors[i], vectors_sizes[i]));
#else   // USE_RAW_POINTER
        std::unique_ptr<uint8_t[]> pdata(new uint8_t[vectors_sizes[i]]);
        memcpy(pdata.get(), vectors[i], vectors_sizes[i]);
        pEndpoint->send(comm::Packet::create(pdata, vectors_sizes[i]));
#endif  // USE_RAW_POINTER
    }

    LOGI("Press enter to check Rx Queue ...\n");
    getchar();

    std::deque<std::unique_ptr<comm::Packet>> pPackets;
    if (pEndpoint->recvAll(pPackets)) {
        if (test(pPackets)) {
            LOGI("-> Passed!\n");
        } else {
            LOGI("-> Failed!\n");
        }
    } else {
        LOGE("Rx Queue is empty!\n");
    }

    LOGI("Press enter to exit ...\n");
    getchar();

    return 0;
}
