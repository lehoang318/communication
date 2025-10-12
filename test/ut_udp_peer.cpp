#include "IP_Endpoint.hpp"
#include "Packet.hpp"
#include "common.hpp"
#include "test_vectors.hpp"
#include "util.hpp"

#include <cinttypes>
#include <cstring>
#include <deque>
#include <string>

int main(int argc, char** argv) {
    if (4 > argc) {
        LOGE("Usage: %s <Local Port> <Peer Address> <Peer Port>\n", argv[0]);
        return 1;
    }

    std::unique_ptr<comm::P2P_Endpoint> pEndpoint = comm::IP_Endpoint::createUdpPeer(
        static_cast<uint16_t>(atoi(argv[1])), std::string(argv[2]), static_cast<uint16_t>(atoi(argv[3])));

    if (!pEndpoint) {
        LOGE("Could not create an Udp Peer which listens at port %s!\n", argv[1]);
        return 1;
    }

    LOGI("Press enter to sent data to peer ...\n");
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
