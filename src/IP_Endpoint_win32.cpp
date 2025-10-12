#include "IP_Endpoint.hpp"

#include <ctime>
#include <memory>
#include <unistd.h>

namespace comm {

int IP_Endpoint::configureSocket(const SOCKET socketFd) {
    BOOL enable = TRUE;
    int ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, (const char*)(&enable), sizeof(enable));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to enable SO_REUSEADDR: %d!\n", WSAGetLastError());
        return -1;
    }

    // winsock does not support timeout!
    // Reference: https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-accept
    unsigned long non_blocking = 1;
    ret = ioctlsocket(socketFd, FIONBIO, &non_blocking);
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to enable NON-BLOCKING mode: %d!\n", WSAGetLastError());
        return -1;
    }

    return 0;
}

ssize_t IP_Endpoint::lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) {
    WSABUF bufferWrapper = {
        .len = (ULONG)limit,
        .buf = (CHAR*)(pBuffer.get())};

    ssize_t byteCount = 0;
    DWORD flags = 0;
    int ret = WSARecvFrom(
        mSocketFd,
        &bufferWrapper, 1,
        (LPDWORD)(&byteCount),
        &flags,  // lpFlags
        NULL,    // lpFrom
        NULL,    // lpFromlen
        NULL,    // lpOverlapped: NULL for Non-overlapped
        NULL     // lpCompletionRoutine: NULL for Non-overlapped
    );

    if (SOCKET_ERROR == ret) {
        if (WSAEWOULDBLOCK == WSAGetLastError()) {
            byteCount = 0;
        } else {
            byteCount = -1;
            mErrorFlag = true;
            LOGE("Failed to read from UDP Socket: %d!\n", WSAGetLastError());
        }
    } else if (0 == byteCount) {
        // Potential: no message is available to be received and the peer has performed an orderly shutdown.
        mErrorFlag = true;
    } else {
        // [TODO] To verify source address against mPeerSockAddr
        LOGD("Received %zd bytes\n", byteCount);
    }

    return byteCount;
}

ssize_t IP_Endpoint::lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
    WSABUF dataWrapper = {
        .len = (ULONG)size,
        .buf = (CHAR*)(pData.get())};

    int ret;
    ssize_t byteCount = 0;
    for (int i = 0; (i < TX_RETRY_LIMIT); i++) {
        ret = WSASendTo(
            mSocketFd,
            &dataWrapper,                            // lpBuffers
            1,                                       // dwBufferCount
            (LPDWORD)(&byteCount),                   // lpNumberOfBytesSent
            0,                                       // dwFlags
            (const struct sockaddr*)&mPeerSockAddr,  // lpTo
            sizeof(mPeerSockAddr),                   // iTolen
            NULL,                                    // lpOverlapped: NULL for Non-overlapped
            NULL                                     // lpCompletionRoutine: NULL for Non-overlapped
        );

        if (0 == ret) {
            if (0 < byteCount) {
                LOGD("Transmitted %zd bytes\n", byteCount);
                break;
            } else if (0 == byteCount) {
                // Should not happen!!!
                LOGW("No data was sent via `WSASendMsg()`!!!\n");
            } else {
                // Should not happen!!!
                mErrorFlag = true;
                byteCount = -1;
                LOGE("Failed to write to Socket: <Unknown>!\n");
                break;
            }
        } else {
            if (WSAEWOULDBLOCK == WSAGetLastError()) {
                // Ignore & retry
                byteCount = 0;
                LOGD("`WSASendMsg()` returned `WSAEWOULDBLOCK`!\n");
            } else {
                byteCount = -1;
                mErrorFlag = true;
                LOGE("Failed to write to Socket: %d!\n", WSAGetLastError());
                break;
            }
        }
        sleep_for(TX_RETRY_BREAK_US);  // [Risk] Shared resources' ownership?
    }

    return byteCount;
}
}  // namespace comm
