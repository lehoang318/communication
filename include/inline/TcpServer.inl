#include "TcpServer.hpp"

namespace comm {

inline TcpServer::TcpServer(SOCKET localSocketFd) {
    mLocalSocketFd = localSocketFd;
    mRxPipeFd = INVALID_SOCKET;
#ifdef __WIN32__
    mTxPipeFd = static_cast<unsigned>(INVALID_SOCKET);
#else   // __WIN32__
    mTxPipeFd = INVALID_SOCKET;
#endif  // __WIN32__

    start();
}

inline TcpServer::~TcpServer() {
    stop();

    if (0 <= mLocalSocketFd) {
#ifdef __WIN32__
        closesocket(mLocalSocketFd);
        WSACleanup();
#else   // __WIN32__
        ::close(mLocalSocketFd);
#endif  // __WIN32__
        mLocalSocketFd = -1;
    }

    LOGI("[%s][%d] Finalized!\n", __func__, __LINE__);
}

inline bool TcpServer::isPeerConnected() {
    return checkTxPipe();
}

inline bool TcpServer::checkRxPipe() {
#ifdef __WIN32__
    return (0 < mRxPipeFd) && (INVALID_SOCKET != mRxPipeFd);
#else   // __WIN32__
    return (0 < mRxPipeFd);
#endif  // __WIN32__
}

inline bool TcpServer::checkTxPipe() {
#ifdef __WIN32__
    return (0 < mRxPipeFd) && (INVALID_SOCKET != mRxPipeFd);
#else   // __WIN32__
    return (0 < mTxPipeFd);
#endif  // __WIN32__
}

}  // namespace comm
