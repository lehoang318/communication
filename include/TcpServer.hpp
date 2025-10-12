#ifndef __TCPSERVER_HPP__
#define __TCPSERVER_HPP__

#include "IP_Endpoint.hpp"

#ifndef __WIN32__
constexpr int INVALID_SOCKET = -1;
#endif  // __WIN32__

namespace comm {

class TcpServer {
   public:
    ~TcpServer() {
        if (0 <= mLocalSocketFd) {
#ifdef __WIN32__
            closesocket(mLocalSocketFd);
            WSACleanup();
#else   // __WIN32__
            ::close(mLocalSocketFd);
#endif  // __WIN32__
        }

        LOGI("Finalized!\n");
    }

    /**
     * @brief Create a new TcpServer object.
     *
     * @param[in] localPort The server shall listen on this port.
     * @return A unique pointer to the TcpServer, or nullptr if an error occurs.
     */
    static std::unique_ptr<TcpServer> create(const uint16_t localPort);

    /**
     * @brief Waiting for connection request.
     *
     * @return P2P_Endpoint object representing the connection with the client.
     */
    std::unique_ptr<P2P_Endpoint> waitForClient(int& errorCode, const long timeout_ms = 1000L);

   protected:
    TcpServer(const SOCKET localSocketFd) {
        mLocalSocketFd = localSocketFd;
    }

   private:
    SOCKET mLocalSocketFd;

    static constexpr int BACKLOG = 5;
    static constexpr long ACCEPT_RETRY_BREAK_MS = 100L;
    static constexpr struct sockaddr_in DUMMY_SOCKADDR = {AF_INET, 0};
};  // class TcpServer

}  // namespace comm

#endif  // __TCPSERVER_HPP__
