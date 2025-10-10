#ifndef __UDPPEER_HPP__
#define __UDPPEER_HPP__

#include "IP_Endpoint.hpp"
#include "Packet.hpp"
#include "common.hpp"

namespace comm {

class UdpPeer : public IP_Endpoint {
   public:
    virtual ~UdpPeer() {}

    /**
     * @brief Create a new UdpPeer object.
     *
     * @param[in] localPort UdpPeer shall listen on this port.
     * @param[in] peerAddress The IP address of the peer.
     * @param[in] peerPort The port on which the peer shall listen.
     * @return A unique pointer to the TcpClient, or nullptr if an error occurs.
     */
    static std::unique_ptr<UdpPeer> create(
        const uint16_t& localPort, const std::string& peerAddress, const uint16_t& peerPort);

   protected:
    UdpPeer(const int& socketFd, const struct sockaddr_in& peerAddress);

};  // class Peer

}  // namespace comm

#include "inline/UdpPeer.inl"

#endif  // __UDPPEER_HPP__
