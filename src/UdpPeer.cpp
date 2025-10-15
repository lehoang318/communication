#include "IP_Endpoint.hpp"
#include "common.hpp"

#include <arpa/inet.h>
#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <string>

namespace comm {

std::unique_ptr<IP_Endpoint> IP_Endpoint::createUdpPeer(const uint16_t& localPort, const std::string& peerAddress, const uint16_t& peerPort) {
    std::unique_ptr<IP_Endpoint> udpPeer;

    if ((0 == localPort) && (0 == peerPort)) {
        LOGE("UDP Ports must be positive!!!\n");
        return udpPeer;
    }

    SOCKET socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (0 > socketFd) {
        LOGE("Could not create UDP socket: %d!!!\n", errno);
        return udpPeer;
    }

    if (0 > IP_Endpoint::configureSocket(socketFd)) {
        ::close(socketFd);
        return udpPeer;
    }

    struct sockaddr_in localSocketAddr;
    localSocketAddr.sin_family = AF_INET;
    localSocketAddr.sin_addr.s_addr = INADDR_ANY;
    localSocketAddr.sin_port = htons(localPort);
    int ret = bind(socketFd, (const struct sockaddr*)(&localSocketAddr), sizeof(localSocketAddr));
    if (0 > ret) {
        ::close(socketFd);
        LOGE("Failed to assigns address to the socket: %d!!!\n", errno);
        return udpPeer;
    }

    if (peerAddress.empty() || (0 == peerPort)) {
        ::close(socketFd);
        LOGE("Invalid peer information: `%s`/%u!\n", peerAddress.c_str(), peerPort);
        return udpPeer;
    }

    struct sockaddr_in remoteSocketAddr;
    remoteSocketAddr.sin_family = AF_INET;
    in_addr_t ipv4_addr = inet_addr(peerAddress.c_str());
    if ((INADDR_NONE == ipv4_addr) || (INADDR_ANY == ipv4_addr)) {
        ::close(socketFd);
        LOGE("Invalid peer address: `%s`!!!\n", peerAddress.c_str());
        return udpPeer;
    }
    remoteSocketAddr.sin_addr.s_addr = ipv4_addr;
    remoteSocketAddr.sin_port = htons(peerPort);

    udpPeer.reset(new IP_Endpoint(socketFd, remoteSocketAddr));

    LOGI("Created new UdpPeer (local port: %u) <=> `%s`/%u.\n", localPort, peerAddress.c_str(), peerPort);

    return udpPeer;
}

}  // namespace comm
