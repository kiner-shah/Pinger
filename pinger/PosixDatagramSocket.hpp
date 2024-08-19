// Copyright (c) 2024, Kiner Shah
// This source code is distributed under BSD-3 Clause License
// details of which can be found in LICENSE file.

#pragma once
#if defined(__unix__)
#include "Socket.hpp"
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>

namespace pinger
{
class PosixDatagramSocket : public Socket
{
    std::int32_t m_sock_fd;
    ::sockaddr_in m_dest_addr;
public:
    PosixDatagramSocket();
    ~PosixDatagramSocket();
    std::system_error connect(const std::uint32_t& destination_address) override;
    std::system_error send(const char* buffer, std::size_t buffer_length, int& bytes_sent) override;
    std::system_error recv(char* buffer, std::size_t buffer_length, int& bytes_recv) override;
    std::system_error disconnect() override;
    bool is_raw_socket() const override;
};
}   // namespace pinger
#endif  // if defined(__unix__)