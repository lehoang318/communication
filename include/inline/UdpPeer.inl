#include "UdpPeer.hpp"

namespace comm {

inline UdpPeer::UdpPeer(
    const SOCKET& socketFd, const uint16_t& localPort,
    const std::string& peerAddress, const uint16_t& peerPort) {

    mSocketFd = socketFd;
    mLocalPort = localPort;

    if (peerAddress.empty() || (0 == peerPort)) {
        mPeerSockAddr.sin_port = 0;
    } else {
        mPeerSockAddr.sin_family = AF_INET;
        mPeerSockAddr.sin_port = htons(peerPort);
        mPeerSockAddr.sin_addr.s_addr = inet_addr(peerAddress.c_str());
    }

    start();
}

inline UdpPeer::~UdpPeer() {
    stop();

    if (0 <= mSocketFd) {
#ifdef __WIN32__
        closesocket(mSocketFd);
        WSACleanup();
#else   // __WIN32__
        ::close(mSocketFd);
#endif  // __WIN32__
        mSocketFd = -1;
    }

    LOGI("[%s][%d] Finalized!\n", __func__, __LINE__);
}

inline bool UdpPeer::checkTxPipe() {
    std::lock_guard<std::mutex> lock(mTxMutex);
    return (0 < mPeerSockAddr.sin_port);
}

}  // namespace comm
