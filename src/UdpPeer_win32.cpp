#include "UdpPeer.hpp"

namespace comm {

std::unique_ptr<UdpPeer> UdpPeer::create(
    const uint16_t& localPort, const std::string& peerAddress, const uint16_t& peerPort) {
    std::unique_ptr<UdpPeer> udpPeer;

    if (0 == localPort) {
        LOGI("Peer 's Port must be a positive value!\n", __func__, __LINE__);
        return udpPeer;
    }

    int ret;

    //---------------------------------------
    // Initialize Winsock
    WSADATA wsaData;
    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (NO_ERROR != ret) {
        LOGE("WSAStartup() failed (error code: %d)!\n", ret);
        return udpPeer;
    }

    SOCKET socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (INVALID_SOCKET == socketFd) {
        LOGE("socket() failed (error code: %d)!\n", WSAGetLastError());
        WSACleanup();
        return udpPeer;
    }

    BOOL enable = TRUE;
    ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to enable SO_REUSEADDR (error code: %d)\n", WSAGetLastError());
        closesocket(socketFd);
        WSACleanup();
        return udpPeer;
    }

    unsigned long non_blocking = 1;
    ret = ioctlsocket(socketFd, FIONBIO, &non_blocking);
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to enable NON-BLOCKING mode (error code: %d)\n", WSAGetLastError());
        closesocket(socketFd);
        WSACleanup();
        return udpPeer;
    }

    struct sockaddr_in localSocketAddr;
    localSocketAddr.sin_family = AF_INET;
    localSocketAddr.sin_addr.s_addr = INADDR_ANY;
    localSocketAddr.sin_port = htons(localPort);
    ret = bind(socketFd, reinterpret_cast<const struct sockaddr*>(&localSocketAddr), sizeof(localSocketAddr));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to assigns address to the socket (error code: %d)\n", WSAGetLastError());
        closesocket(socketFd);
        WSACleanup();
        return udpPeer;
    }

    LOGI("Bound at port %u\n", __func__, __LINE__, localPort);
    return std::unique_ptr<UdpPeer>(new UdpPeer(socketFd, localPort, peerAddress, peerPort));
}

bool UdpPeer::setDestination(const std::string& address, const uint16_t& port) {
    if (address.empty() || (0 == port)) {
        LOGI("Invalid peer information (`%s`/%u)!\n", __func__, __LINE__, address.c_str(), port);
        return false;
    }

    std::lock_guard<std::mutex> lock(mTxMutex);
    mPeerSockAddr.sin_family = AF_INET;
    mPeerSockAddr.sin_port = htons(port);
    mPeerSockAddr.sin_addr.s_addr = inet_addr(address.c_str());

    return true;
}

ssize_t UdpPeer::lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) {
    socklen_t remoteAddressSize;
    struct sockaddr_in remoteSocketAddr;
    remoteAddressSize = (socklen_t)sizeof(remoteSocketAddr);

    ssize_t ret = recvfrom(
        mSocketFd, reinterpret_cast<char*>(pBuffer.get()), limit, 0,
        reinterpret_cast<struct sockaddr*>(&remoteSocketAddr), &remoteAddressSize);

    if (SOCKET_ERROR == ret) {
        int error = WSAGetLastError();
        if (WSAEWOULDBLOCK == error) {
            ret = 0;
        } else {
            LOGE("Failed to read from UDP Socket (error code: %d)\n", error);
        }
    } else if (0 == ret) {
        // zero-length datagrams!
        LOGI("Zero-length datagram!\n", __func__, __LINE__);
    } else {
        LOGD("Received %zd bytes\n", __func__, __LINE__, ret);
    }

    return ret;
}

ssize_t UdpPeer::lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
    // Send data over UDP
    ssize_t ret = 0LL;
    for (int i = 0; i < TX_RETRY_COUNT; i++) {
        {
            std::lock_guard<std::mutex> lock(mTxMutex);
            ret = sendto(
                mSocketFd, reinterpret_cast<const char*>(pData.get()), size, 0,
                reinterpret_cast<struct sockaddr*>(&mPeerSockAddr), sizeof(mPeerSockAddr));

            if (SOCKET_ERROR == ret) {
                int error = WSAGetLastError();
                if (WSAEWOULDBLOCK == error) {
                    // Ignore & retry
                } else {
                    LOGE("Failed to write to UDP Socket (error code: %d)\n", error);
                }
            } else if (0 == ret) {
                // Should not happen!
            } else {
                LOGD("Transmitted %zd bytes\n", __func__, __LINE__, ret);
                break;
            }
        }
    }

    return ret;
}

}  // namespace comm
