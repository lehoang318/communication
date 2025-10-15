#include "IP_Endpoint.hpp"
#include "common.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace comm {

std::unique_ptr<IP_Endpoint> IP_Endpoint::createTcpClient(const std::string& serverAddr, const uint16_t& remotePort) {
    std::unique_ptr<IP_Endpoint> tcpClient;

    if (serverAddr.empty()) {
        LOGE("Server 's Address is invalid!!!\n");
        return tcpClient;
    }

    if (0 == remotePort) {
        LOGE("Server 's Port must be a positive value!\n");
        return tcpClient;
    }

    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (NO_ERROR != ret) {
        LOGE("WSAStartup() failed: %d!!!\n", ret);
        return tcpClient;
    }

    SOCKET socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == socketFd) {
        WSACleanup();
        LOGE("Could not create TCP socket: %d!!!\n", WSAGetLastError());
        return tcpClient;
    }

    if (0 > IP_Endpoint::configureSocket(socketFd)) {
        closesocket(socketFd);
        WSACleanup();
        return tcpClient;
    }

    struct sockaddr_in remoteSocketAddr;
    remoteSocketAddr.sin_family = AF_INET;
    unsigned long ipv4_addr = inet_addr(serverAddr.c_str());
    if ((INADDR_NONE == ipv4_addr) || (INADDR_ANY == ipv4_addr)) {
        closesocket(socketFd);
        WSACleanup();
        LOGE("Invalid server address: `%s`!!!\n", serverAddr.c_str());
        return tcpClient;
    }
    remoteSocketAddr.sin_addr.s_addr = ipv4_addr;
    remoteSocketAddr.sin_port = htons(remotePort);

    int errorCode = 0;
    const auto deadline = monotonic_now() + std::chrono::seconds(RX_TIMEOUT_S);
    do {
        ret = connect(socketFd, (const struct sockaddr*)(&remoteSocketAddr), sizeof(remoteSocketAddr));
        if (0 == ret) {
            LOGI("Connected to %s/%u.", serverAddr.c_str(), remotePort);
            break;
        } else {
            // Reference: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-connect#return-value
            errorCode = WSAGetLastError();
            if (WSAEISCONN == errorCode) {
                // Socket is already connected.
                ret = 0;
                break;
            } else if ((WSAEWOULDBLOCK == errorCode) || (WSAEALREADY == errorCode) || (WSAEINVAL == errorCode)) {
                sleep_for(CONNECT_RETRY_BREAK_US);
            } else {
                break;
            }
        }
    } while (deadline > monotonic_now());

    if (SOCKET_ERROR == ret) {
        closesocket(socketFd);
        WSACleanup();
        LOGE("Failed to connect to %s/%u: %d!!!\n", serverAddr.c_str(), remotePort, errorCode);
        return tcpClient;
    }

    tcpClient.reset(new IP_Endpoint(socketFd, remoteSocketAddr));

    LOGI("Connected to %s/%u.\n", serverAddr.c_str(), remotePort);

    return tcpClient;
}

}  // namespace comm
