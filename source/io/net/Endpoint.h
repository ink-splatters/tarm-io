/*----------------------------------------------------------------------------------------------
 *  Copyright (c) 2020 - present Alexander Voitenko
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *----------------------------------------------------------------------------------------------*/

#pragma once

#include "../CommonMacros.h"
#include "../Export.h"
#include "../Forward.h"

#include <array>
#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace tarm {
namespace io {
namespace net {

class Endpoint {
public:
    friend class TcpClient;
    friend class TcpServer;
    friend class UdpClient;
    friend class UdpPeer;
    friend class UdpServer;

    struct sockaddr_placeholder {};

    TARM_IO_DECLARE_DLL_PUBLIC_MOVE(Endpoint);;

    TARM_IO_DLL_PUBLIC Endpoint& operator=(const Endpoint& other);
    TARM_IO_DLL_PUBLIC Endpoint(const Endpoint& other);

    enum Type {
        UNDEFINED = 0,
        IP_V4,
        IP_V6
    };

    TARM_IO_DLL_PUBLIC Endpoint();
    TARM_IO_DLL_PUBLIC Endpoint(const std::string& address, std::uint16_t port);
    TARM_IO_DLL_PUBLIC Endpoint(std::uint32_t address, std::uint16_t port);
    TARM_IO_DLL_PUBLIC Endpoint(unsigned long address, std::uint16_t port);
    TARM_IO_DLL_PUBLIC Endpoint(const std::uint8_t* address_bytes, std::size_t address_size, std::uint16_t port);
    TARM_IO_DLL_PUBLIC Endpoint(std::initializer_list<std::uint8_t> address_bytes, std::uint16_t port);
    TARM_IO_DLL_PUBLIC Endpoint(std::uint8_t (&address_bytes)[4], std::uint16_t port);
    TARM_IO_DLL_PUBLIC Endpoint(std::uint8_t (&address_bytes)[16], std::uint16_t port);
    TARM_IO_DLL_PUBLIC Endpoint(const sockaddr_placeholder* raw_address);
    TARM_IO_DLL_PUBLIC ~Endpoint();

    TARM_IO_DLL_PUBLIC std::string address_string() const;
    TARM_IO_DLL_PUBLIC std::uint16_t port() const;

    TARM_IO_DLL_PUBLIC Type type() const;

    TARM_IO_DLL_PUBLIC std::uint32_t ipv4_addr() const;

    TARM_IO_DLL_PUBLIC void clear();

private:
    void* raw_endpoint();
    const void* raw_endpoint() const;

    class Impl;
    std::unique_ptr<Impl> m_impl;
};

TARM_IO_DLL_PUBLIC
std::ostream& operator <<(std::ostream& o, const Endpoint& e);

} // namespace net
} // namespace io
} // namespace tarm
