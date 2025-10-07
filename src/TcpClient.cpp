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

    SOCKET socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > socketFd) {
        perror("Could not create TCP socket!\n");
        return tcpClient;
    }

    int enable = 1;
    ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    if (0 > ret) {
        perror("Failed to enable SO_REUSEADDR!\n");
        ::close(socketFd);
        return tcpClient;
    }

    int flags = fcntl(socketFd, F_GETFL, 0);
    if (0 > flags) {
        perror("Failed to get socket flags!\n");
        ::close(socketFd);
        return tcpClient;
    }

    if (0 > fcntl(socketFd, F_SETFL, (flags | O_NONBLOCK))) {
        perror("Failed to enable NON-BLOCKING mode!\n");
        ::close(socketFd);
        return tcpClient;
    }

    struct sockaddr_in remoteSocketAddr;
    remoteSocketAddr.sin_family = AF_INET;
    remoteSocketAddr.sin_addr.s_addr = inet_addr(serverAddr.c_str());
    remoteSocketAddr.sin_port = htons(remotePort);

    auto t0 = get_monotonic_clock();

    do {
        ret = connect(socketFd, reinterpret_cast<const struct sockaddr*>(&remoteSocketAddr), sizeof(remoteSocketAddr));
        if (0 == ret) {
            break;
        }
    } while (
        RX_TIMEOUT_S > std::chrono::duration_cast<std::chrono::seconds>(
                           get_monotonic_clock() - t0)
                           .count());

    if (0 != ret) {
        perror("Failed to connect to server!\n");
        ::close(socketFd);
        return tcpClient;
    }

    LOGI("[%s][%d] Connected to %s/%u\n", __func__, __LINE__, serverAddr.c_str(), remotePort);

    return std::unique_ptr<TcpClient>(new TcpClient(socketFd, serverAddr, remotePort));
}

void TcpClient::runRx() {
    while (!mExitFlag) {
        if (!proceedRx()) {
            LOGE("[%s][%d] Rx Pipe was broken!\n", __func__, __LINE__);
            break;
        }
    }
}

void TcpClient::runTx() {
    while (!mExitFlag) {
        if (!proceedTx()) {
            LOGE("[%s][%d] Tx Pipe was broken!\n", __func__, __LINE__);
            break;
        }
    }
}

ssize_t TcpClient::lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) {
    ssize_t ret = ::recv(mSocketFd, pBuffer.get(), limit, 0);
    if (0 > ret) {
        if (EWOULDBLOCK == errno) {
            ret = 0;
        } else {
            perror("Failed to read from TCP Socket!");
        }
    } else if (0 == ret) {
        ret = -2;  // Stream socket peer has performed an orderly shutdown!
    } else {
        LOGD("[%s][%d] Received %zd bytes\n", __func__, __LINE__, ret);
    }

    return ret;
}

ssize_t TcpClient::lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
    // Send data over TCP
    ssize_t ret = 0LL;
    for (int i = 0; i < TX_RETRY_COUNT; i++) {
        ret = ::send(mSocketFd, pData.get(), size, 0);
        if (0 > ret) {
            if (EWOULDBLOCK == errno) {
                // Ignore & retry
            } else {
                perror("Failed to write to TCP Socket!");
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
