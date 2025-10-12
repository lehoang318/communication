#include "IP_Endpoint.hpp"
#include "TcpServer.hpp"
#include "common.hpp"

namespace comm {

constexpr struct sockaddr_in TcpServer::DUMMY_SOCKADDR;

std::unique_ptr<TcpServer> TcpServer::create(const uint16_t localPort) {
    std::unique_ptr<TcpServer> tcpServer;

    if (0 == localPort) {
        LOGI("Local Port must be a positive value!\n");
        return tcpServer;
    }

    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (NO_ERROR != ret) {
        LOGE("WSAStartup() failed: %d!\n", ret);
        return tcpServer;
    }

    SOCKET socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == socketFd) {
        WSACleanup();
        LOGE("Could not create TCP socket: %d!\n", WSAGetLastError());
        return tcpServer;
    }

    if (0 > IP_Endpoint::configureSocket(socketFd)) {
        closesocket(socketFd);
        WSACleanup();
        return tcpServer;
    }

    struct sockaddr_in socketAddr;
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_addr.s_addr = INADDR_ANY;
    socketAddr.sin_port = htons(localPort);
    ret = bind(socketFd, (const struct sockaddr *)(&socketAddr), sizeof(socketAddr));
    if (SOCKET_ERROR == ret) {
        closesocket(socketFd);
        WSACleanup();
        LOGE("Failed to assigns address to the socket: %d!\n", WSAGetLastError());
        return tcpServer;
    }

    ret = listen(socketFd, BACKLOG);
    if (SOCKET_ERROR == ret) {
        closesocket(socketFd);
        WSACleanup();
        LOGE("Failed to mark the socket as a passive socket: %d!\n", WSAGetLastError());
        return tcpServer;
    }

    tcpServer.reset(new TcpServer(socketFd));

    LOGI("TCP Server is listenning at port %u ...\n", localPort);

    return tcpServer;
}

std::unique_ptr<P2P_Endpoint> TcpServer::waitForClient(int &errorCode, const long timeout_ms) {
    std::unique_ptr<IP_Endpoint> clientEndpoint;
    struct sockaddr_in remoteSocketAddr;
    int remoteAddressSize = (int)sizeof(remoteSocketAddr);

    const auto deadline = monotonic_now() + std::chrono::milliseconds(timeout_ms);
    SOCKET socketFd;
    errorCode = 0;
    do {
        socketFd = accept(
            mLocalSocketFd,
            (struct sockaddr *)(&remoteSocketAddr),
            &remoteAddressSize);

        if (INVALID_SOCKET == socketFd) {
            if (WSAEWOULDBLOCK == WSAGetLastError()) {
                sleep_for(ACCEPT_RETRY_BREAK_MS * US_PER_MS);
            } else {
                errorCode = WSAGetLastError();
                LOGE("Encountered errors when executing `accept()`: %d!\n", WSAGetLastError());
                return clientEndpoint;
            }
        } else {
            break;
        }
    } while (deadline > monotonic_now());

    if (INVALID_SOCKET == socketFd) {
        LOGI("No pending connection.");
        return clientEndpoint;
    }

    unsigned long non_blocking = 1;
    if (SOCKET_ERROR == ioctlsocket(socketFd, FIONBIO, &non_blocking)) {
        errorCode = WSAGetLastError();
        closesocket(socketFd);
        LOGE("Failed to enable NON-BLOCKING mode: %d!\n", WSAGetLastError());
        return clientEndpoint;
    }

    clientEndpoint.reset(new IP_Endpoint(socketFd, DUMMY_SOCKADDR));

    return clientEndpoint;
}

}  // namespace comm
