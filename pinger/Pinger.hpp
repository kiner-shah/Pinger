// Copyright (c) 2024, Kiner Shah
// This source code is distributed under BSD-3 Clause License
// details of which can be found in LICENSE file.

#pragma once
#include <functional>
#include <atomic>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include "Socket.hpp"

namespace pinger
{

struct PingerConfig
{
    std::string destination_address;
    // Number of ping requests to send, 0 means infinite
    std::uint32_t count = 0;
};

class Pinger
{
    using PingerCallbackOnNetworkChange = std::function<void(bool)>;
    using LoopCondition = std::function<bool(void)>;

    PingerConfig m_conf;
    PingerCallbackOnNetworkChange m_callback;
    LoopCondition m_loop_condition;

    std::condition_variable m_cv;
    std::mutex m_mutex;
    std::unique_ptr<Socket> m_socket;
    std::thread m_receive_thread;
    std::thread m_send_thread;

    std::chrono::time_point<std::chrono::steady_clock> m_request_start_time;

    std::uint32_t m_loop_counter;
    const std::uint32_t m_source_ip_address;
    const std::uint32_t m_dest_ip_address;
    const std::uint16_t m_identifier;

    std::atomic_int16_t m_sequence_number {0};
    std::atomic_bool m_is_stopped {false};
    std::atomic_bool m_is_request_sent{ false };
    bool m_is_network_available {false};

    bool handle_receive();
    void handle_send();

public:
    Pinger(const PingerConfig& conf, PingerCallbackOnNetworkChange callback);
    ~Pinger();
    void start();
    void stop();
};
}   // namespace pinger