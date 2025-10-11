#ifndef __IP_ENDPOINT_HPP__
#define __IP_ENDPOINT_HPP__

#include "P2P_Endpoint.hpp"

#include <atomic>
#include <memory>

#ifdef __WIN32__
#include <WinDef.h>
#include <winsock2.h>  // Need to link with Ws2_32.lib

#else  // __WIN32__
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef int SOCKET;
#endif  // __WIN32__

namespace comm {

class IP_Endpoint : public P2P_Endpoint {
   public:
    IP_Endpoint(const SOCKET& socketFd, const struct sockaddr_in& peerAddress) {
        mSocketFd = socketFd;
        mPeerSockAddr = peerAddress;
        start();
    }

    virtual ~IP_Endpoint() {
        stop();

        if (0 <= mSocketFd) {
#ifdef __WIN32__
            closesocket(mSocketFd);
            WSACleanup();
#else   // __WIN32__
            ::close(mSocketFd);
#endif  // __WIN32__
        }

        LOGI("Finalized!\n");
    }

    /**
     * @brief Create a new UdpPeer object.
     *
     * @param[in] localPort UdpPeer shall listen on this port.
     * @param[in] peerAddress The IP address of the peer.
     * @param[in] peerPort The port on which the peer shall listen.
     * @return A unique pointer to the TcpClient, or nullptr if an error occurs.
     */
    static std::unique_ptr<IP_Endpoint> createUdpPeer(const uint16_t& localPort, const std::string& peerAddress, const uint16_t& peerPort);
    
    /**
     * @brief Create a new TcpClient object.
     *
     * @param[in] serverAddr The IP address of the server.
     * @param[in] remotePort The port number of the server.
     * @return A unique pointer to the TcpClient, or nullptr if an error occurs.
     */
    static std::unique_ptr<IP_Endpoint> createTcpClient(const std::string& serverAddr, const uint16_t& remotePort);

    /**
     * @brief Configure socket to allow Reuse of local addresses (SO_REUSEADDR) and Non-blocking I/O.
     * 
     * @return 0 on success, otherwise -1.
     */
    static int configureSocket(const SOCKET socketFd);

   protected:
    bool checkRxPipe() override {
        return !mErrorFlag;
    }

    bool checkTxPipe() override {
        return !mErrorFlag;
    }

    ssize_t lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) override;
    ssize_t lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) override;

   private:
    SOCKET mSocketFd;
    struct sockaddr_in mPeerSockAddr;

    std::atomic<bool> mErrorFlag {false};
};  // class Peer

}  // namespace comm

#endif // __IP_ENDPOINT_HPP__
