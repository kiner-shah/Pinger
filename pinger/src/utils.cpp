// Copyright (c) 2024, Kiner Shah
// This source code is distributed under BSD-3 Clause License
// details of which can be found in LICENSE file.

#include "utils.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(__unix__)
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

namespace pinger
{
std::uint16_t network_to_host_short(std::uint16_t val)
{
#if defined(_WIN32)
    return ntohs(val);
#elif defined(__unix__)
    return ::ntohs(val);
#endif
}

std::uint16_t host_to_network_short(std::uint16_t val)
{
#if defined(_WIN32)
    return htons(val);
#elif defined(__unix__)
    return ::htons(val);
#endif
}

std::uint32_t network_to_host_long(std::uint32_t val)
{
#if defined(_WIN32)
    return ntohl(val);
#elif defined(__unix__)
    return ::ntohl(val);
#endif
}

std::uint32_t host_to_network_long(std::uint32_t val)
{
#if defined(_WIN32)
    return htonl(val);
#elif defined(__unix__)
    return ::htonl(val);
#endif
}

std::uint32_t get_ip_address(const std::string &address_str)
{
#if defined(_WIN32)
    WSADATA wsa_data;
    auto ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (ret != 0) {
        return 0;
    }
    addrinfo hints;
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;

    addrinfo* result;
    ret = getaddrinfo(address_str.c_str(), "echo", &hints, &result);
    if ( ret != 0 ) {
        WSACleanup();
        return 0;
    }
    std::uint32_t ret_address = 0;
    for (::addrinfo* p = result; p != nullptr; p = p->ai_next)
    {
        auto sck = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sck == INVALID_SOCKET)
        {
            continue;
        }
        if (connect(sck, p->ai_addr, p->ai_addrlen) == 0)
        {
            closesocket(sck);
            sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(p->ai_addr);
            std::cerr << address_str << " resolved to address: " << inet_ntoa(addr->sin_addr) << '\n';
            ret_address = addr->sin_addr.S_un.S_addr;
        }
        closesocket(sck);
    }
    WSACleanup();
    return ret_address;
#elif defined(__unix__)
    ::addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_protocol = 0;
    hints.ai_socktype = SOCK_DGRAM;

    ::addrinfo* result;
    auto ret = ::getaddrinfo(address_str.c_str(), "echo", &hints, &result);
    if (ret < 0)
    {
        return 0;
    }
    std::uint32_t ret_address = 0;
    for (::addrinfo* p = result; p != nullptr; p = p->ai_next)
    {
        int sfd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sfd < 0)
        {
            continue;
        }
        if (::connect(sfd, p->ai_addr, p->ai_addrlen) == 0)
        {
            ::close(sfd);
            ::sockaddr_in* addr = reinterpret_cast<::sockaddr_in*>(p->ai_addr);
            std::cerr << address_str << " resolved to address: " << ::inet_ntoa(addr->sin_addr) << '\n';
            ret_address = addr->sin_addr.s_addr;
            break;
        }
        ::close(sfd);
    }
    ::freeaddrinfo(result);
    return ret_address;
#endif
}

std::uint16_t get_process_id()
{
#if defined(_WIN32)
    return static_cast<std::uint16_t>(::GetCurrentProcessId());
#elif defined(__unix__)
    return static_cast<std::uint16_t>(::getpid());
#endif
}

std::string get_ip_string(std::uint32_t ip_address)
{
#if defined(_WIN32)
    sockaddr_in addr;
    addr.sin_addr.S_un.S_addr = ip_address;
    return std::string(inet_ntoa(addr.sin_addr));
#elif defined(__unix__)
    ::sockaddr_in addr;
    addr.sin_addr.s_addr = ip_address;
    return std::string(::inet_ntoa(addr.sin_addr));
#endif
}

}   // namespace pinger
