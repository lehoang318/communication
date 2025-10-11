#include "IP_Endpoint.hpp"

#include <ctime>
#include <memory>

#include <fcntl.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>

namespace comm {

int IP_Endpoint::configureSocket(const SOCKET socketFd) {
    int enable = 1;
    int ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    if (0 > ret) {
        LOGE("Failed to enable SO_REUSEADDR: %d!\n", errno);
        return -1;
    }

    int flags = fcntl(socketFd, F_GETFL, 0);
    if (0 > flags) {
        LOGE("Failed to get socket flags: %d!\n", errno);
        return -1;
    }

    ret = fcntl(socketFd, F_SETFL, (flags | O_NONBLOCK));
    if (0 > ret) {
        LOGE("Failed to enable NON-BLOCKING mode: %d!\n", errno);
        return -1;
    }

    return 0;
}

ssize_t IP_Endpoint::lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) {
    ssize_t ret = recvfrom(
        mSocketFd,
        pBuffer.get(),
        limit,
        0,              // flags
        NULL,           // address
        NULL            // address_len
    );
    if (0 > ret) {
        if (EWOULDBLOCK == errno) {
            ret = 0;
        } else {
            mErrorFlag = true;
            LOGE("Failed to read from Socket: %d!\n", errno);
        }
    } else if (0 == ret) {
        // Should not happen!!!
        LOGW("Zero-length payload!\n");
    } else {
        // [TODO] To verify source address against mPeerSockAddr
        LOGD("Received %zd bytes\n", ret);
    }

    return ret;
}

ssize_t IP_Endpoint::lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
    ssize_t ret = 0;
    for (int i = 0; (i < TX_RETRY_LIMIT); i++) {
        ret = sendto(
            mSocketFd,
            pData.get(),
            size,
            0,                                          // flags
            (const struct sockaddr *)(&mPeerSockAddr),  // dest_address
            sizeof(mPeerSockAddr)                       // dest_address_len
        );
        if (0 < ret) {
            LOGD("Transmitted %zd bytes\n", ret);
            break;
        } else if (0 == ret) {
            // Should not happen!!!
            LOGW("No data was sent via `sendmsg()`!!!\n");
        } else if (EWOULDBLOCK == errno) {
            ret = 0;
            LOGD("`sendmsg()` returned `EWOULDBLOCK`!\n");
        } else {
            mErrorFlag = true;
            LOGE("Failed to write to Socket: %d!\n", errno);
            break;
        }

        sleep_for(TX_RETRY_BREAK_US);   // [Risk] Shared resources' ownership?
    }

    return ret;
}
}  // namespace comm
