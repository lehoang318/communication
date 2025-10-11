#include "TcpServer.hpp"
#include "IP_Endpoint.hpp"
#include "common.hpp"

#include <fcntl.h>

namespace comm {

std::unique_ptr<TcpServer> TcpServer::create(const uint16_t localPort) {
    std::unique_ptr<TcpServer> tcpServer;

    if (0 == localPort) {
        LOGI("Local Port must be a positive value!\n");
        return tcpServer;
    }

    SOCKET socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > socketFd) {
        LOGE("Could not create TCP socket: %d!\n", errno);
        return tcpServer;
    }

    if (0 > IP_Endpoint::configureSocket(socketFd)) {
        ::close(socketFd);
		return tcpServer;
    }

    struct sockaddr_in socketAddr;
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_addr.s_addr = INADDR_ANY;
    socketAddr.sin_port = htons(localPort);
    int ret = bind(socketFd, (const struct sockaddr *)(&socketAddr), sizeof(socketAddr));
    if (0 > ret) {
        ::close(socketFd);
        LOGE("Failed to assigns address to the socket: %d!\n", errno);
        return tcpServer;
    }

    ret = listen(socketFd, BACKLOG);
    if (0 != ret) {
        ::close(socketFd);
        LOGE("Failed to mark the socket as a passive socket: %d!\n", errno);
        return tcpServer;
    }

    tcpServer.reset(new TcpServer(socketFd));

    LOGI("TCP Server is listenning at port %u ...\n", localPort);

    return tcpServer;
}

std::unique_ptr<IP_Endpoint> TcpServer::waitForClient(int& errorCode, const long timeout_ms) {
    std::unique_ptr<IP_Endpoint> clientEndpoint;
    struct sockaddr_in remoteSocketAddr;
    socklen_t remoteAddressSize = (socklen_t)sizeof(remoteSocketAddr);

    const auto deadline = monotonic_now() + std::chrono::milliseconds(timeout_ms);
    int socketFd;
    errorCode = 0;
    do {
        socketFd = accept(
            mLocalSocketFd,
            (struct sockaddr *)(&remoteSocketAddr),
            &remoteAddressSize
        );

        if (0 < socketFd) {
            break;
        } else if (0 == socketFd) {
            // Should not happen!!!
            LOGW("`accept()` returned 0!!!\n");
            return clientEndpoint;
        } else if (EWOULDBLOCK == errno) {
            sleep_for(ACCEPT_RETRY_BREAK_MS * US_PER_MS);
        } else {
            errorCode = errno;
            LOGE("Encountered errors when executing `accept()`: %d!\n", errno);
            return clientEndpoint;
        }
    } while (deadline > monotonic_now());

    if (0 >= socketFd) {
        LOGI("No pending connection.");
        return clientEndpoint;
    }

    int flags = fcntl(socketFd, F_GETFL, 0);
    if (0 > flags) {
        ::close(socketFd);
        LOGE("Failed to get socket flags: %d!\n", errno);
        return clientEndpoint;
    }

    if (0 > fcntl(socketFd, F_SETFL, (flags | O_NONBLOCK))) {
        errorCode = errno;
        ::close(socketFd);
        LOGE("Failed to enable NON-BLOCKING mode: %d!\n", errno);
        return clientEndpoint;
    }

    clientEndpoint.reset(new IP_Endpoint(socketFd, DUMMY_SOCKADDR));

    return clientEndpoint;
}

}  // namespace comm
