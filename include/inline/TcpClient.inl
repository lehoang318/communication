#include "TcpClient.hpp"

namespace comm {

inline TcpClient::TcpClient(const SOCKET& socketFd, const std::string serverAddr, uint16_t remotePort) {
    mSocketFd = socketFd;

    mServerAddress = serverAddr;
    mRemotePort = remotePort;

    start();
}

inline TcpClient::~TcpClient() {
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

    LOGI("Finalized!\n", __func__, __LINE__);
}

}  // namespace comm
