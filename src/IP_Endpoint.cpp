#include "IP_Endpoint.hpp"

#include <ctime>
#include <cunistd>
#include <memory>

#include <netinet/in.h>
#include <arpa/inet.h>

#define CMSG_BUF_SIZE (128)

namespace comm {

ssize_t IP_Endpoint::lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) {
    struct iovec bufferWrapper = {
        .iov_base = pBuffer.get(),
        .iov_len = limit
    };
    char control_buf[CMSG_BUF_SIZE];

    struct msghdr msg = {
        .msg_iov = &bufferWrapper,    // Scatter/gather array
        .msg_iovlen = 1,
        .msg_name = NULL,    // [TODO]
        .msg_namelen = 0,
        .msg_control = control_buf,  // Ancillary data
        .msg_controllen = sizeof(control_buf)
    };

    ssize_t ret = recvmsg(mSocketFd, &msg, 0);
    if (0 > ret) {
        if (EWOULDBLOCK == errno) {
            ret = 0;
        } else {
            LOGE("Failed to read from UDP Socket: %d!\n", errno);
        }
    } else if (0 == ret) {
        LOGW("Zero-length datagram!\n");
    } else {
        // [TODO] To verify source address against mPeerSockAddr
        LOGD("Received %zd bytes\n", ret);
    }

    return ret;
}

ssize_t IP_Endpoint::lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
    struct iovec dataWrapper = {
        .iov_base = pData.get(),
        .iov_len = size
    };
    struct msghdr msg = {
        .msg_iov = &dataWrapper,    // Scatter/gather array
        .msg_iovlen = 1,
        .msg_control = NULL, // Ancillary data (not used)
        .msg_controllen = 0
    };

    if (0 == mPeerSockAddr.sin_port)= {
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
    } else {
        msg.msg_name = &mPeerSockAddr;
        msg.msg_namelen = sizeof(mPeerSockAddr);
    }

    ssize_t ret = 0;
    for (int i = 0; (i < TX_RETRY_COUNT); i++) {
        ret = sendmsg(mSocketFd, &msg, 0);
        if (0 < ret) {
            LOGD("Transmitted %zd bytes\n", ret);
            break;
        } else if (0 == ret) {
            // Should not happen!!!
            LOGW("`sendmsg()` returned 0!!!\n");
        }   else if (EWOULDBLOCK == errno) {
            LOGD("`sendmsg()` returned `EWOULDBLOCK`!\n");
        } else {
            mErrorFlag = true;
            LOGE("Failed to write to UDP Socket: %d!\n", errno);
            break;
        }

        sleep_for(0, TX_RETRY_BREAK_US);    // [Risk] Shared resources' ownership?
    }

    return ret;
}
}  // namespace comm
