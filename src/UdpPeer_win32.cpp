#include "IP_Endpoint.hpp"
#include "common.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace comm {

std::unique_ptr<IP_Endpoint> IP_Endpoint::createUdpPeer(const uint16_t& localPort, const std::string& peerAddress, const uint16_t& peerPort) {
    std::unique_ptr<IP_Endpoint> udpPeer;

    if ((0 == localPort) && (0 == peerPort)) {
        LOGE("UDP Ports must be positive!\n");
        return udpPeer;
    }

    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (NO_ERROR != ret) {
        LOGE("WSAStartup() failed: %d!\n", ret);
        return udpPeer;
    }

    SOCKET socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (INVALID_SOCKET == socketFd) {
        WSACleanup();
        LOGE("Could not create UDP socket: %d!\n", WSAGetLastError());
        return udpPeer;
    }

    if (0 > IP_Endpoint::configureSocket(socketFd)) {
        closesocket(socketFd);
        WSACleanup();
        return udpPeer;
    }

    struct sockaddr_in localSocketAddr;
    localSocketAddr.sin_family = AF_INET;
    localSocketAddr.sin_addr.s_addr = INADDR_ANY;
    localSocketAddr.sin_port = htons(localPort);
    ret = bind(socketFd, (const struct sockaddr *)(&localSocketAddr), sizeof(localSocketAddr));
    if (SOCKET_ERROR == ret) {
        closesocket(socketFd);
        WSACleanup();
        LOGE("Failed to assigns address to the socket: %d!\n", WSAGetLastError());
        return udpPeer;
    }

    if (peerAddress.empty() || (0 == peerPort)) {
        closesocket(socketFd);
        WSACleanup();
        LOGE("Invalid peer information: `%s`/%u!\n", peerAddress.c_str(), peerPort);
        return udpPeer;
    }

    struct sockaddr_in remoteSocketAddr;
    remoteSocketAddr.sin_family = AF_INET;
    unsigned long ipv4_addr = inet_addr(peerAddress.c_str());
    if ((INADDR_NONE == ipv4_addr) || (INADDR_ANY == ipv4_addr)) {
        closesocket(socketFd);
        WSACleanup();
        LOGE("Invalid peer address: `%s`!\n", peerAddress.c_str());
        return udpPeer;
    }
    remoteSocketAddr.sin_addr.s_addr = ipv4_addr;
    remoteSocketAddr.sin_port = htons(peerPort);

    udpPeer.reset(new IP_Endpoint(socketFd, remoteSocketAddr));

    LOGI("Created new UdpPeer (local port: %u) <=> `%s`/%u\n", localPort, peerAddress.c_str(), peerPort);

    return udpPeer;
}

}  // namespace comm
