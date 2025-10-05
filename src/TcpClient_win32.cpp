#include "TcpClient.hpp"

namespace comm {

std::unique_ptr<TcpClient> TcpClient::create(const std::string& serverAddr, const uint16_t& remotePort) {
    std::unique_ptr<TcpClient> tcpClient;

    if (serverAddr.empty()) {
        LOGE("[%s][%d] Server 's Address is invalid!\n", __func__, __LINE__);
        return tcpClient;
    }

    if (0 == remotePort) {
        LOGE("[%s][%d] Server 's Port must be a positive value!\n", __func__, __LINE__);
        return tcpClient;
    }

    int ret;

    //---------------------------------------
    // Initialize Winsock
    WSADATA wsaData;
    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (NO_ERROR != ret) {
        LOGE("WSAStartup() failed (error code: %d)!\n", ret);
        return tcpClient;
    }

    SOCKET socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == socketFd) {
        LOGE("socket() failed (error code: %d)!\n", WSAGetLastError());
        WSACleanup();
        return tcpClient;
    }

    BOOL enable = TRUE;
    ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&enable), sizeof(enable));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to enable SO_REUSEADDR (error code: %d)\n", WSAGetLastError());
        closesocket(socketFd);
        WSACleanup();
        return tcpClient;
    }

    unsigned long non_blocking = 1;
    ret = ioctlsocket(socketFd, FIONBIO, &non_blocking);
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to enable NON-BLOCKING mode (error code: %d)\n", WSAGetLastError());
        closesocket(socketFd);
        WSACleanup();
        return tcpClient;
    }

    struct sockaddr_in remoteSocketAddr;
    remoteSocketAddr.sin_family      = AF_INET;
    remoteSocketAddr.sin_addr.s_addr = inet_addr(serverAddr.c_str());
    remoteSocketAddr.sin_port        = htons(remotePort);

    auto t0 = get_monotonic_clock();

    do {
        ret = connect(socketFd, reinterpret_cast<const struct sockaddr *>(&remoteSocketAddr), sizeof(remoteSocketAddr));
        if ((0 == ret) || (WSAEISCONN == WSAGetLastError())) {
            ret = 0;
            break;
        }
    } while (
        RX_TIMEOUT_S > std::chrono::duration_cast<std::chrono::seconds>(
                            get_monotonic_clock() - t0
                        ).count()
    );

    if (SOCKET_ERROR == ret) {
        LOGE("Failed to connect to %s/%u (error code: %d)\n", serverAddr.c_str(), remotePort, WSAGetLastError());
        closesocket(socketFd);
        WSACleanup();
        return tcpClient;
    }

    LOGI("[%s][%d] Connected to %s/%u\n", __func__, __LINE__, serverAddr.c_str(), remotePort);

    return std::unique_ptr<TcpClient>(new TcpClient(socketFd, serverAddr, remotePort));
}

void TcpClient::close() {
    mExitFlag = true;

    if ((mpRxThread) && (mpRxThread->joinable())) {
        mpRxThread->join();
    }

    if ((mpTxThread) && (mpTxThread->joinable())) {
        mpTxThread->join();
    }

    if (0 <= mSocketFd) {
        closesocket(mSocketFd);
        WSACleanup();
        mSocketFd = -1;
    }
}

void TcpClient::runRx() {
    while(!mExitFlag) {
        if (!proceedRx()) {
            LOGE("[%s][%d] Rx Pipe was broken!\n", __func__, __LINE__);
            break;
        }
    }
}

void TcpClient::runTx() {
    while(!mExitFlag) {
        if (!proceedTx()) {
            LOGE("[%s][%d] Tx Pipe was broken!\n", __func__, __LINE__);
            break;
        }
    }
}

ssize_t TcpClient::lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) {
    ssize_t ret = ::recv(mSocketFd, reinterpret_cast<char *>(pBuffer.get()), limit, 0);
    if (SOCKET_ERROR == ret) {
        int error = WSAGetLastError();
        if (WSAEWOULDBLOCK == error) {
            ret = 0;
        } else {
            LOGE("Failed to read from TCP Socket (error code: %d)\n", error);
        }
    } else if (0 == ret) {
        ret = -2;   // Stream socket peer has performed an orderly shutdown!
    } else {
        LOGD("[%s][%d] Received %zd bytes\n", __func__, __LINE__, ret);
    }

    return ret;
}

ssize_t TcpClient::lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
    // Send data over TCP
    ssize_t ret = 0LL;
    for (int i = 0; i < TX_RETRY_COUNT; i++) {
        ret = ::send(mSocketFd, reinterpret_cast<const char *>(pData.get()), size, 0);
        if (SOCKET_ERROR == ret) {
            int error = WSAGetLastError();
            if (WSAEWOULDBLOCK == error) {
                // Ignore & retry
            } else {
                LOGE("Failed to write to TCP Socket (error code: %d)\n", error);
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

}   // namespace comm
