// Copyright (c) 2024, Kiner Shah
// This source code is distributed under BSD-3 Clause License
// details of which can be found in LICENSE file.

#if defined(__unix__)
#include "PosixDatagramSocket.hpp"
#include "utils.hpp"
#include <cstring>
#include <system_error>
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <arpa/inet.h>

namespace
{
static std::error_code get_error_code(int linux_errno)
{
    std::errc ec;
    switch (linux_errno)
    {
    case EACCES: ec = std::errc::permission_denied; break;
    case EADDRINUSE: ec = std::errc::address_in_use; break;
    case EADDRNOTAVAIL: ec = std::errc::address_not_available; break;
    case EAGAIN: ec = std::errc::resource_unavailable_try_again; break;
    case EFAULT: ec = std::errc::bad_address; break;
    case EINPROGRESS: ec = std::errc::operation_in_progress; break;
    case EINVAL: ec = std::errc::invalid_argument; break;
    case ENOBUFS: ec = std::errc::no_buffer_space; break;
    case ENOTSOCK: ec = std::errc::not_a_socket; break;
    case ENETDOWN: ec = std::errc::network_down; break;
    case EINTR: ec = std::errc::interrupted; break;
    case EALREADY: ec = std::errc::connection_already_in_progress; break;
    case EAFNOSUPPORT: ec = std::errc::address_family_not_supported; break;
    case ECONNREFUSED: ec = std::errc::connection_refused; break;
    case EISCONN: ec = std::errc::already_connected; break;
    case ENOTCONN: ec = std::errc::not_connected; break;
    case ENETUNREACH: ec = std::errc::network_unreachable; break;
    case ENETRESET: ec = std::errc::network_reset; break;
    case EHOSTUNREACH: ec = std::errc::host_unreachable; break;
    case ETIMEDOUT: ec = std::errc::timed_out; break;
#if EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK: ec = std::errc::operation_would_block; break;
#endif
    case EOPNOTSUPP: ec = std::errc::operation_not_supported; break;
    case EMSGSIZE: ec = std::errc::message_size; break;
    case ECONNABORTED: ec = std::errc::connection_aborted; break;
    case ECONNRESET: ec = std::errc::connection_reset; break;
    case EDESTADDRREQ: ec = std::errc::destination_address_required; break;
    case EBADF: ec = std::errc::bad_file_descriptor; break;
    case ELOOP: ec = std::errc::too_many_symbolic_link_levels; break;
    case ENAMETOOLONG: ec = std::errc::filename_too_long; break;
    case ENOENT: ec = std::errc::no_such_file_or_directory; break;
    case ENOMEM: ec = std::errc::not_enough_memory; break;
    case ENOTDIR: ec = std::errc::not_a_directory; break;
    case EROFS: ec = std::errc::read_only_file_system; break;
    case EPROTOTYPE: ec = std::errc::wrong_protocol_type; break;
    case EPERM: ec = std::errc::operation_not_permitted; break;
    case EPIPE: ec = std::errc::broken_pipe; break;
    case EIO: ec = std::errc::io_error; break;

    default:
        return std::error_code(linux_errno, std::system_category());
    }
    return std::make_error_code(ec);
}
}   // namespace
namespace pinger
{
PosixDatagramSocket::PosixDatagramSocket()
{
    m_sock_fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (m_sock_fd < 0)
    {
        std::cerr << "Socket creation failed [" << errno << "] " << strerror(errno) << '\n';
        exit(EXIT_FAILURE);
    }

    ::timeval tv;
    tv.tv_sec = 5;  // 5 seconds
    tv.tv_usec = 0;
    int ret = ::setsockopt(m_sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (ret < 0)
    {
        std::cerr << "Socket set opt failed [" << errno << "] " << strerror(errno) << '\n';
        ::close(m_sock_fd);
        exit(EXIT_FAILURE);
    }
}

PosixDatagramSocket::~PosixDatagramSocket()
{
    if (m_sock_fd > 0)
    {
        ::close(m_sock_fd);
    }
}

std::system_error PosixDatagramSocket::connect(const std::uint32_t& destination_address)
{
    m_dest_addr.sin_family = AF_INET;
    m_dest_addr.sin_addr.s_addr = destination_address;
    m_dest_addr.sin_port = host_to_network_short(0);
    std::cerr << "Connect to " << inet_ntoa(m_dest_addr.sin_addr) << '\n';

    ::sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = get_process_id();
    addr.sin_addr.s_addr = INADDR_ANY;

    // ICMP header identifier is the port that the socket has bound to.
    // We want to use current process id for ICMP header identifier,
    // so we set the port above to current process id. And then we bind
    // to that port.
    auto ret = ::bind(m_sock_fd, reinterpret_cast<::sockaddr*>(&addr), sizeof(addr));
    if (ret < 0)
    {
        return std::system_error(get_error_code(errno), ::strerror(errno));
    }

    ret = ::connect(m_sock_fd, reinterpret_cast<::sockaddr*>(&m_dest_addr), sizeof(m_dest_addr));
    if (ret < 0)
    {
        return std::system_error(get_error_code(errno), ::strerror(errno));
    }
    return std::system_error(std::error_code());
}

std::system_error PosixDatagramSocket::send(const char* buffer, std::size_t buffer_length, int& bytes_sent)
{
    bytes_sent = ::sendto(m_sock_fd, buffer, buffer_length, 0, reinterpret_cast<::sockaddr*>(&m_dest_addr), sizeof(m_dest_addr));
    if (bytes_sent < 0)
    {
        return std::system_error(get_error_code(errno), ::strerror(errno));
    }
    return std::system_error(std::error_code());
}

std::system_error PosixDatagramSocket::recv(char *buffer, std::size_t buffer_length, int& bytes_recv)
{
    ::sockaddr recv_from_address;
    ::socklen_t recv_from_address_length;
    bytes_recv = ::recvfrom(m_sock_fd, buffer, buffer_length, MSG_WAITALL, &recv_from_address, &recv_from_address_length);
    if (bytes_recv < 0)
    {
        // Socket is blocking, and on recv timeout either EAGAIN or EWOULDBLOCK is thrown
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return std::system_error(std::make_error_code(std::errc::timed_out), "timed out");
        }
        return std::system_error(get_error_code(errno), ::strerror(errno));
    }
    return std::system_error(std::error_code());
}

std::system_error PosixDatagramSocket::disconnect()
{
    auto ret = ::shutdown(m_sock_fd, SHUT_RDWR);
    if (ret < 0)
    {
        return std::system_error(get_error_code(errno), ::strerror(errno));
    }
    return std::system_error(std::error_code());
}

bool PosixDatagramSocket::is_raw_socket() const
{
    return false;
}
}   // namespace pinger
#endif  // if defined(__unix__)