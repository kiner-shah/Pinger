// Copyright (c) 2024, Kiner Shah
// This source code is distributed under BSD-3 Clause License
// details of which can be found in LICENSE file.

#include "Socket.hpp"
#if defined(_WIN32)
#include "WindowsRawSocket.hpp"
#elif defined(__unix__)
#include "PosixDatagramSocket.hpp"
#endif

namespace pinger
{
std::unique_ptr<Socket> create_socket()
{
#if defined(_WIN32)
    return std::make_unique<WindowsRawSocket>();
#elif defined(__unix__)
    return std::make_unique<PosixDatagramSocket>();
#endif
}
}   // namespace pinger