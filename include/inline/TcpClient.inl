#include "TcpClient.hpp"

namespace comm {

inline TcpClient::TcpClient(const SOCKET& socketFd, const std::string serverAddr, uint16_t remotePort) {
    mExitFlag = false;

    mSocketFd = socketFd;

    mServerAddress = serverAddr;
    mRemotePort = remotePort;

    mpRxThread.reset(new std::thread(&TcpClient::runRx, this));
    mpTxThread.reset(new std::thread(&TcpClient::runTx, this));
}

inline TcpClient::~TcpClient() {
    mExitFlag = true;

    if ((mpRxThread) && (mpRxThread->joinable())) {
        mpRxThread->join();
    }

    if ((mpTxThread) && (mpTxThread->joinable())) {
        mpTxThread->join();
    }

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

}  // namespace comm
