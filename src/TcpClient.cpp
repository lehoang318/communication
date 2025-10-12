#include "IP_Endpoint.hpp"
#include "common.hpp"

#include <arpa/inet.h>
#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <string>

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

    SOCKET socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > socketFd) {
        LOGE("Could not create TCP socket: %d!\n", errno);
        return tcpClient;
    }

    if (0 > IP_Endpoint::configureSocket(socketFd)) {
        ::close(socketFd);
        return tcpClient;
    }

    struct sockaddr_in remoteSocketAddr;
    remoteSocketAddr.sin_family = AF_INET;
    in_addr_t ipv4_addr = inet_addr(serverAddr.c_str());
    if ((INADDR_NONE == ipv4_addr) || (INADDR_ANY == ipv4_addr)) {
        ::close(socketFd);
        LOGE("Invalid server address: `%s`!\n", serverAddr.c_str());
        return tcpClient;
    }
    remoteSocketAddr.sin_addr.s_addr = ipv4_addr;
    remoteSocketAddr.sin_port = htons(remotePort);

    int ret;
    const auto deadline = monotonic_now() + std::chrono::seconds(RX_TIMEOUT_S);
    do {
        ret = connect(socketFd, (const struct sockaddr*)(&remoteSocketAddr), sizeof(remoteSocketAddr));
        if (0 == ret) {
            break;
        } else {
            if ((EINPROGRESS == errno) || (EAGAIN == errno)) {
                sleep_for(CONNECT_RETRY_BREAK_US);
            } else {
                break;
            }
        }
    } while (deadline > monotonic_now());

    if (0 != ret) {
        ::close(socketFd);
        LOGE("Failed to connect to %s/%u: %d\n", serverAddr.c_str(), remotePort, errno);
        return tcpClient;
    }

    tcpClient.reset(new IP_Endpoint(socketFd, remoteSocketAddr));

    LOGI("Connected to %s/%u\n", serverAddr.c_str(), remotePort);

    return tcpClient;
}

}  // namespace comm
