// Copyright (c) 2024, Kiner Shah
// This source code is distributed under BSD-3 Clause License
// details of which can be found in LICENSE file.

#pragma once
#include <functional>
#include <memory>
#include <string>
#include <cstdint>

namespace pinger
{

struct PingerConfig
{
    std::string destination_address;
    // Number of ping requests to send, 0 means infinite
    std::uint32_t count = 0;
};

using PingerCallbackOnNetworkChange = std::function<void(bool)>;

class Pinger
{
public:
    static std::unique_ptr<Pinger> create(const PingerConfig& conf, PingerCallbackOnNetworkChange callback);
    virtual ~Pinger() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
};
}   // namespace pinger