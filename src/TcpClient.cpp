#include "IP_Endpoint.hpp"
#include "common.hpp"

#include <cstdint>
#include <memory>
#include <string>

#include <netinet/in.h>
#include <arpa/inet.h>

namespace comm {

std::unique_ptr<IP_Endpoint> IP_Endpoint::createTcpClient(const std::string& serverAddr, const uint16_t& remotePort) {
    std::unique_ptr<IP_Endpoint> tcpClient;

    if (serverAddr.empty()) {
        LOGE("Server 's Address is invalid!\n");
        return tcpClient;
    }

    if (0 == remotePort) {
        LOGE("Server 's Port must be a positive value!\n");
        return tcpClient;
    }

    int ret;

    SOCKET socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > socketFd) {
        LOGE("Could not create TCP socket: %d!\n", errno);
        return tcpClient;
    }

    int enable = 1;
    ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    if (0 > ret) {
        LOGE("Failed to enable SO_REUSEADDR: %d!\n", errno);
        ::close(socketFd);
        return tcpClient;
    }

    int flags = fcntl(socketFd, F_GETFL, 0);
    if (0 > flags) {
        LOGE("Failed to get socket flags: %d!\n", errno);
        ::close(socketFd);
        return tcpClient;
    }

    ret = fcntl(socketFd, F_SETFL, (flags | O_NONBLOCK));
    if (0 > ret) {
        LOGE("Failed to enable NON-BLOCKING mode: %d!\n", errno);
        ::close(socketFd);
        return tcpClient;
    }

    struct sockaddr_in remoteSocketAddr;
    remoteSocketAddr.sin_family = AF_INET;
    ret = inet_aton(serverAddr.c_str(), &remoteSocketAddr.sin_addr);
    if (0 == ret) {
        LOGE("Invalid server address: `%s`!\n", serverAddr.c_str());
        ::close(socketFd);
        return tcpClient;
    }
    remoteSocketAddr.sin_port = htons(remotePort);

    auto deadline = monotonic_now() + std::chrono::seconds(RX_TIMEOUT_S);
    do {
        ret = connect(socketFd, reinterpret_cast<const struct sockaddr*>(&remoteSocketAddr), sizeof(remoteSocketAddr));
    } while ((deadline > monotonic_now()) && (0 != ret));

    if (0 != ret) {
        LOGE("Failed to connect to server: %d!\n", errno);
        ::close(socketFd);
        return tcpClient;
    }

    tcpClient.reset(new IP_Endpoint(socketFd, remoteSocketAddr));

    LOGI("Connected to %s/%u\n", serverAddr.c_str(), remotePort);

    return tcpClient;
}

}  // namespace comm
