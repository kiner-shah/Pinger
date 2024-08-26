// Copyright (c) 2024, Kiner Shah
// This source code is distributed under BSD-3 Clause License
// details of which can be found in LICENSE file.

#if defined(_WIN32)
#include "WindowsRawSocket.hpp"
#include "utils.hpp"
#include <system_error>
#include <iostream>
#include <array>

namespace pinger
{
namespace
{
static std::string get_error_message_from_error_code(int win_error_code)
{
    std::array<char, 256> message_buffer;
    std::fill(message_buffer.begin(), message_buffer.end(), 0);

    FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
            nullptr,                                     // lpsource
            win_error_code,                              // message id
            MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  // languageid
            message_buffer.data(),                       // output buffer
            message_buffer.size(),                       // size of msgbuf, bytes
            nullptr);                                    // va_list of arguments
    
    return std::string(message_buffer.begin(), message_buffer.end());
}

static std::error_code get_error_code(int win_error_code)
{
    std::errc ec;
    switch (win_error_code)
    {
    case WSAEACCES: ec = std::errc::permission_denied; break;
    case WSAEADDRINUSE: ec = std::errc::address_in_use; break;
    case WSAEADDRNOTAVAIL: ec = std::errc::address_not_available; break;
    case WSAEFAULT: ec = std::errc::bad_address; break;
    case WSAEINPROGRESS: ec = std::errc::operation_in_progress; break;
    case WSAEINVAL: ec = std::errc::invalid_argument; break;
    case WSAENOBUFS: ec = std::errc::no_buffer_space; break;
    case WSAENOTSOCK: ec = std::errc::not_a_socket; break;
    case WSAENETDOWN: ec = std::errc::network_down; break;
    case WSAEINTR: ec = std::errc::interrupted; break;
    case WSAEALREADY: ec = std::errc::connection_already_in_progress; break;
    case WSAEAFNOSUPPORT: ec = std::errc::address_family_not_supported; break;
    case WSAECONNREFUSED: ec = std::errc::connection_refused; break;
    case WSAEISCONN: ec = std::errc::already_connected; break;
    case WSAENOTCONN: ec = std::errc::not_connected; break;
    case WSAENETUNREACH: ec = std::errc::network_unreachable; break;
    case WSAENETRESET: ec = std::errc::network_reset; break;
    case WSAEHOSTUNREACH: ec = std::errc::host_unreachable; break;
    case WSAETIMEDOUT: ec = std::errc::timed_out; break;
    case WSAEWOULDBLOCK: ec = std::errc::operation_would_block; break;
    case WSAEOPNOTSUPP: ec = std::errc::operation_not_supported; break;
    case WSAEMSGSIZE: ec = std::errc::message_size; break;
    case WSAECONNABORTED: ec = std::errc::connection_aborted; break;
    case WSAECONNRESET: ec = std::errc::connection_reset; break;
    case WSAEDESTADDRREQ: ec = std::errc::destination_address_required; break;

    default:
        return std::error_code(win_error_code, std::system_category());
    }
    return std::make_error_code(ec);
}

}   // namespace
WindowsRawSocket::WindowsRawSocket()
    : m_dest_addr{}
{
    WSADATA wsa_data;
    int ret = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if (ret != 0)
    {
        std::cerr << "WSAStartup failed with error [" << ret << "] " << get_error_message_from_error_code(ret);
        exit(EXIT_FAILURE);
    }

    // Windows doesn't seem to support IPPROTO_ICMP with SOCK_DGRAM, thus used raw sockets.
    m_sock = ::socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (m_sock == INVALID_SOCKET)
    {
        int err = WSAGetLastError();
        std::cerr << "Socket creation failed [" << err << "] " << get_error_message_from_error_code(err) << '\n';
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    DWORD timeout_in_millis = 5000; // 5 seconds
    ret = ::setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout_in_millis), sizeof(timeout_in_millis));
    if (ret == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        std::cerr << "Socket set opt failed [" << err << "] " << get_error_message_from_error_code(err) << '\n';
        ::closesocket(m_sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
}

WindowsRawSocket::~WindowsRawSocket()
{
    if (m_sock != INVALID_SOCKET)
    {
        ::closesocket(m_sock);
        WSACleanup();
    }
}

std::system_error WindowsRawSocket::connect(const std::uint32_t &destination_address)
{
    m_dest_addr.sin_family = AF_INET;
    m_dest_addr.sin_addr.S_un.S_addr = destination_address;
    m_dest_addr.sin_port = host_to_network_short(0);
    std::cerr << "Connect to " << inet_ntoa(m_dest_addr.sin_addr) << '\n';

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = host_to_network_short(get_process_id());
    addr.sin_addr.S_un.S_addr = host_to_network_long(INADDR_ANY);

    // ICMP header identifier is the port that the socket has bound to.
    // We want to use current process id for ICMP header identifier,
    // so we set the port above to current process id. And then we bind
    // to that port.
    auto ret = ::bind(m_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (ret == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        return std::system_error(get_error_code(err), get_error_message_from_error_code(err));
    }

    ret = ::connect(m_sock, reinterpret_cast<sockaddr*>(&m_dest_addr), sizeof(m_dest_addr));
    if (ret == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        return std::system_error(get_error_code(err), get_error_message_from_error_code(err));
    }
    return std::system_error(std::error_code());
}

std::system_error WindowsRawSocket::send(const char *buffer, std::size_t buffer_length, int &bytes_sent)
{
    auto ret = ::sendto(m_sock, buffer, buffer_length, 0, reinterpret_cast<sockaddr*>(&m_dest_addr), sizeof(m_dest_addr));
    if (ret == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        return std::system_error(get_error_code(err), get_error_message_from_error_code(err));
    }
    return std::system_error(std::error_code());
}

std::system_error WindowsRawSocket::recv(char *buffer, std::size_t buffer_length, int &bytes_recv)
{
    sockaddr recv_from_address;
    socklen_t recv_from_address_length;
    // Windows support recvfrom for only datagrams, thus used recv.
    auto ret = ::recv(m_sock, buffer, buffer_length, 0);
    if (ret == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        return std::system_error(get_error_code(err), get_error_message_from_error_code(err));
    }
    bytes_recv = ret;
    return std::system_error(std::error_code());
}

std::system_error WindowsRawSocket::disconnect()
{
    // Note: If recv is running and shutdown is called, recv continues to block (in contrast to Posix
    // where recv throws some error).
    auto ret = ::shutdown(m_sock, SD_BOTH);
    if (ret == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        return std::system_error(get_error_code(err), get_error_message_from_error_code(err));
    }
    return std::system_error(std::error_code());
}

bool WindowsRawSocket::is_raw_socket() const
{
    return true;
}
} // namespace pinger
#endif  // if defined(_WIN32)