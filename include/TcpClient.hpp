#ifndef __TCPCLIENT_HPP__
#define __TCPCLIENT_HPP__

#include "P2P_Endpoint.hpp"
#include "Packet.hpp"
#include "common.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>

#ifdef __WIN32__
#include <winsock2.h>  // Need to link with Ws2_32.lib

#else  // __WIN32__
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#endif  // __WIN32__

#ifndef __WIN32__
typedef int SOCKET;
#endif  // __WIN32__

namespace comm {

class TcpClient : public P2P_Endpoint {
   public:
    void close() override;

    virtual ~TcpClient();

    /**
     * @brief Create a new TcpClient object.
     *
     * @param[in] serverAddr The IP address of the server.
     * @param[in] remotePort The port number of the server.
     * @return A unique pointer to the TcpClient, or nullptr if an error occurs.
     */
    static std::unique_ptr<TcpClient> create(const std::string& serverAddr, const uint16_t& remotePort);

   protected:
    TcpClient(const SOCKET& socketFd, const std::string serverAddr, uint16_t remotePort);

    ssize_t lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) override;
    ssize_t lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) override;

   private:
    /**
     * @brief Rx thread.
     */
    void runRx();

    /**
     * @brief Tx thread.
     */
    void runTx();

    std::string mServerAddress;
    uint16_t mRemotePort;
    SOCKET mSocketFd;

    std::unique_ptr<std::thread> mpRxThread;
    std::unique_ptr<std::thread> mpTxThread;
    std::atomic<bool> mExitFlag;
};  // class TcpClient

}  // namespace comm

#include "inline/TcpClient.inl"

#endif  // __TCPCLIENT_HPP__
