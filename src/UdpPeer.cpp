#include "IP_Endpoint.hpp"
#include "common.hpp"

#include <cstdint>
#include <memory>
#include <string>

#include <netinet/in.h>
#include <arpa/inet.h>

namespace comm {

std::unique_ptr<IP_Endpoint> IP_Endpoint::createUdpPeer(const uint16_t& localPort, const std::string& peerAddress, const uint16_t& peerPort) {
    std::unique_ptr<IP_Endpoint> udpPeer;

    if ((0 == localPort) && (0 == peerPort)) {
        LOGE("UDP Ports must be positive!\n");
        return udpPeer;
    }

    int socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (0 > socketFd) {
        LOGE("Could not create UDP socket: %d!\n", errno);
        return udpPeer;
    }

    int enable = 1;
    int ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    if (0 > ret) {
        LOGE("Failed to enable SO_REUSEADDR: %d!\n", errno);
        ::close(socketFd);
        return udpPeer;
    }

    int flags = fcntl(socketFd, F_GETFL, 0);
    if (0 > flags) {
        LOGE("Failed to get socket flags: %d!\n", errno);
        ::close(socketFd);
        return udpPeer;
    }

    ret = fcntl(socketFd, F_SETFL, (flags | O_NONBLOCK));
    if (0 > ret) {
        LOGE("Failed to enable NON-BLOCKING mode: %d!\n", errno);
        ::close(socketFd);
        return udpPeer;
    }

    struct sockaddr_in localSocketAddr;
    localSocketAddr.sin_family = AF_INET;
    localSocketAddr.sin_addr.s_addr = INADDR_ANY;
    localSocketAddr.sin_port = htons(localPort);
    ret = bind(socketFd, reinterpret_cast<const struct sockaddr*>(&localSocketAddr), sizeof(localSocketAddr));
    if (0 > ret) {
        LOGE("Failed to assigns address to the socket: %d!\n", errno);
        ::close(socketFd);
        return udpPeer;
    }

    if (address.empty() || (0 == port)) {
        LOGI("Invalid peer information (`%s`/%u)!\n", __func__, __LINE__, address.c_str(), port);
        ::close(socketFd);
        return false;
    }

    struct sockaddr_in peerSockAddr;
    peerSockAddr.sin_family = AF_INET;
    ret = inet_aton(peerAddress.c_str(), &peerSockAddr.sin_addr);
    if (0 == ret) {
        LOGE("Invalid peer address: `%s`!\n", peerAddress.c_str());
        return udpPeer;
    }
    peerSockAddr.sin_port = htons(peerPort);

    udpPeer.reset(new IP_Endpoint(socketFd, peerSockAddr));

    LOGI("Created new UdpPeer (local port: %u)\n", localPort);

    return udpPeer;
}

}  // namespace comm
