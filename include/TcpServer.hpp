#ifndef __TCPSERVER_HPP__
#define __TCPSERVER_HPP__

#include "P2P_Endpoint.hpp"
#include "Packet.hpp"
#include "common.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <unistd.h>

#ifdef __WIN32__
#include <winsock2.h>  // Need to link with Ws2_32.lib
#include <ws2tcpip.h>

#else  // __WIN32__
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#endif  // __WIN32__

#ifndef __WIN32__
typedef int SOCKET;
constexpr int INVALID_SOCKET = -1;
#endif  // __WIN32__

namespace comm {

class TcpServer : public P2P_Endpoint {
   public:
    bool isPeerConnected() override;
    void close() override;

    ~TcpServer();

    /**
     * @brief Create a new TcpServer object.
     *
     * @param[in] localPort The server shall listen on this port.
     * @return A unique pointer to the TcpServer, or nullptr if an error occurs.
     */
    static std::unique_ptr<TcpServer> create(uint16_t localPort);

   protected:
    TcpServer(SOCKET localSocketFd);

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

    /**
     * @brief Check status of file descriptor of Rx channel.
     */
    bool checkRxPipe();

    /**
     * @brief Check status of file descriptor of Tx channel.
     */
    bool checkTxPipe();

    int mLocalSocketFd;

    /**
     * @note TcpServer accept only one client at a time, therefore, accept & read take place in the same thread
     *          -> no need to use std::atomic for Rx Pipe
     */
    SOCKET mRxPipeFd;

#ifdef __WIN32__
    std::atomic<long long unsigned int> mTxPipeFd;
#else   // __WIN32__
    std::atomic<int> mTxPipeFd;
#endif  // __WIN32__

    std::unique_ptr<std::thread> mpRxThread;
    std::unique_ptr<std::thread> mpTxThread;
    std::atomic<bool> mExitFlag;

    static constexpr int BACKLOG = 1;
};  // class TcpServer

}  // namespace comm

#include "inline/TcpServer.inl"

#endif  // __TCPSERVER_HPP__
