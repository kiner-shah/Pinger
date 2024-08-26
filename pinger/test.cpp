// Copyright (c) 2024, Kiner Shah
// This source code is distributed under BSD-3 Clause License
// details of which can be found in LICENSE file.

#include "Pinger.hpp"
#include <string>
#include <iostream>
#include <thread>

int main()
{
    using namespace std::chrono_literals;

    pinger::PingerConfig config;
    config.count = 20;
    config.destination_address = "google.com";

    auto callback = [](bool is_online) {
        std::cerr << (is_online ? "ONLINE" : "OFFLINE") << '\n';
    };

    auto pinger_ptr = pinger::Pinger::create(config, callback);
    pinger_ptr->start();
    std::this_thread::sleep_for(10s);
    pinger_ptr->stop();
}
