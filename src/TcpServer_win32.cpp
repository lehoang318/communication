#include "TcpServer.hpp"

namespace comm {

std::unique_ptr<TcpServer> TcpServer::create(uint16_t localPort) {
    std::unique_ptr<TcpServer> tcpServer;

    if (0 == localPort) {
        LOGE("[%s][%d] Local Port must be a positive value!\n", __func__, __LINE__);
        return tcpServer;
    }

    int ret;

    //---------------------------------------
    // Initialize Winsock
    WSADATA wsaData;
    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (NO_ERROR != ret) {
        LOGE("WSAStartup() failed (error code: %d)!\n", ret);
        return tcpServer;
    }

    SOCKET localSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == localSocketFd) {
        LOGE("socket() failed (error code: %d)!\n", WSAGetLastError());
        WSACleanup();
        return tcpServer;
    }

    BOOL enable = TRUE;
    ret = setsockopt(localSocketFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to enable SO_REUSEADDR (error code: %d)\n", WSAGetLastError());
        closesocket(localSocketFd);
        WSACleanup();
        return tcpServer;
    }

    // winsock does not support timeout!
    // Reference: https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-accept
    unsigned long non_blocking = 1;
    if (SOCKET_ERROR == ioctlsocket(localSocketFd, FIONBIO, &non_blocking)) {
        LOGE("Failed to enable NON-BLOCKING mode (error code: %d)\n", WSAGetLastError());
        closesocket(localSocketFd);
        WSACleanup();
        return tcpServer;
    }

    struct sockaddr_in localSocketAddr;
    localSocketAddr.sin_family = AF_INET;
    localSocketAddr.sin_addr.s_addr = INADDR_ANY;
    localSocketAddr.sin_port = htons(localPort);
    ret = bind(localSocketFd, reinterpret_cast<const struct sockaddr*>(&localSocketAddr), sizeof(localSocketAddr));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to assigns address to the socket (error code: %d)\n", WSAGetLastError());
        closesocket(localSocketFd);
        WSACleanup();
        return tcpServer;
    }

    ret = listen(localSocketFd, BACKLOG);
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to mark the socket as a passive socket (error code: %d)\n", WSAGetLastError());
        closesocket(localSocketFd);
        WSACleanup();
        return tcpServer;
    }

    LOGI("[%s][%d] TCP Server is listenning at port %u ...\n", __func__, __LINE__, localPort);

    return std::unique_ptr<TcpServer>(new TcpServer(localSocketFd));
}

void TcpServer::close() {
    mExitFlag = true;

    if ((mpRxThread) && (mpRxThread->joinable())) {
        mpRxThread->join();
    }

    if ((mpTxThread) && (mpTxThread->joinable())) {
        mpTxThread->join();
    }

    if (0 <= mLocalSocketFd) {
        closesocket(mLocalSocketFd);
        WSACleanup();
        mLocalSocketFd = -1;
    }
}

void TcpServer::runRx() {
    struct sockaddr_in remoteSocketAddr;
    socklen_t remoteAddressSize = static_cast<socklen_t>(sizeof(remoteSocketAddr));

    while (!mExitFlag) {
        mRxPipeFd = accept(
            mLocalSocketFd,
            reinterpret_cast<struct sockaddr*>(&remoteSocketAddr),
            &remoteAddressSize);

        if (!checkRxPipe()) {
            if (WSAEWOULDBLOCK == WSAGetLastError()) {
                // Timeout - do nothing
                continue;
            } else {
                perror("Could not access connection queue!\n");
                break;
            }
        }

        unsigned long non_blocking = 1;
        if (SOCKET_ERROR == ioctlsocket(mRxPipeFd, FIONBIO, &non_blocking)) {
            LOGE("Failed to enable NON-BLOCKING mode (error code: %d)\n", WSAGetLastError());
            closesocket(mRxPipeFd);
            WSACleanup();
            continue;
        }

        mTxPipeFd = mRxPipeFd;

        while (!mExitFlag) {
            if (!proceedRx()) {
                LOGE("[%s][%d] Rx Pipe was broken!\n", __func__, __LINE__);
                break;
            }
        }

        mTxPipeFd = INVALID_SOCKET;
        closesocket(mRxPipeFd);
        mRxPipeFd = INVALID_SOCKET;
    }
}

void TcpServer::runTx() {
    while (!mExitFlag) {
        if (!proceedTx(!checkTxPipe())) {
            LOGE("[%s][%d] Tx Pipe was broken!\n", __func__, __LINE__);
        }
    }
}

ssize_t TcpServer::lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) {
    ssize_t ret = ::recv(mRxPipeFd, reinterpret_cast<char*>(pBuffer.get()), limit, 0);
    if (SOCKET_ERROR == ret) {
        int error = WSAGetLastError();
        if (WSAEWOULDBLOCK == error) {
            ret = 0;
        } else {
            LOGE("Failed to read from TCP Socket (error code: %d)\n", error);
        }
    } else if (0 == ret) {
        ret = -2;  // Stream socket peer has performed an orderly shutdown!
    } else {
        LOGD("[%s][%d] Received %zd bytes\n", __func__, __LINE__, ret);
    }

    return ret;
}

ssize_t TcpServer::lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
    // Send data over TCP
    ssize_t ret = 0LL;
    for (int i = 0; i < TX_RETRY_COUNT; i++) {
        ret = ::send(mTxPipeFd, reinterpret_cast<const char*>(pData.get()), size, 0);
        if (SOCKET_ERROR == ret) {
            int error = WSAGetLastError();
            if (WSAEWOULDBLOCK == error) {
                // Ignore & retry
            } else {
                mTxPipeFd = -1;
                LOGE("Failed to write to TCP Socket (error code: %d)\n", error);
                break;
            }
        } else if (0 == ret) {
            // Should not happen!
        } else {
            LOGD("[%s][%d] Transmitted %zd bytes\n", __func__, __LINE__, ret);
            break;
        }
    }

    return ret;
}

}  // namespace comm
