// Copyright (c) 2024, Kiner Shah
// This source code is distributed under BSD-3 Clause License
// details of which can be found in LICENSE file.

#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <ostream>
#include <istream>

namespace pinger
{
constexpr std::uint8_t IPV4_HEADER_SIZE_EXCLUDING_OPTIONS_IN_BYTES = 20;
constexpr std::uint8_t IPV4_HEADER_OPTIONS_MAX_SIZE_IN_BYTES = 40;
constexpr std::uint8_t IPV4_HEADER_MAX_TOTAL_SIZE = 60;

constexpr std::uint8_t IPV4_PROTOCOL_NUMBER_ICMP = 1;
// For Network operations, administration and management (OAM), example: ping, ssh, telnet, etc.
constexpr std::uint8_t IPV4_DSCP_CS2 = 16;

// Default value for version is 4
constexpr std::uint8_t IPv4_VERSION_DEFAULT = 4;
// Default value for IHL is 5 (5 * 4 = 20 bytes)
constexpr std::uint8_t IPv4_IHL_DEFAULT = 5;
constexpr std::uint8_t IPV4_VERSION_IHL_DEFAULT = (IPv4_VERSION_DEFAULT << 4) | IPv4_IHL_DEFAULT;

struct alignas(4) ipv4_header
{
    using OptionsArray = std::array<std::uint8_t, IPV4_HEADER_OPTIONS_MAX_SIZE_IN_BYTES>;
    using OptionsVector = std::vector<std::uint8_t>;

    // first 4-bits contain version, rest 4-bits contain IHL (Internet Header Length)
    std::uint8_t version_and_ihl = IPV4_VERSION_IHL_DEFAULT;
    // first 6-bits contain DSCP (Differentiated Services Code Point), rest 2-bits contain ECN (Explicit Congestion Notification)
    std::uint8_t dscp_and_ecn = (IPV4_DSCP_CS2 << 2);
    std::uint16_t total_length = 0;
    std::uint16_t identification = 0;
    // first 3-bits contain flags, rest 13-bits contain fragment offset
    // Flags are in order (left-to-right): More Fragments, Don't Fragment and Reserved (always 0)
    std::uint16_t flags_and_fragment_offset = 0;
    std::uint8_t time_to_live = 0;
    std::uint8_t protocol = 0;
    std::uint16_t header_checksum = 0;
    std::uint32_t source_address = 0;
    std::uint32_t destination_address = 0;
    // Options are usually like this
    // Option Code - 8 bytes (Copied, Option Class, Option Number bits)
    // Option Length - 8 bytes (optional)
    // Option Data Value - variable length (optional, based on option length)
    OptionsArray options = {0};

    void compute_and_set_header_checksum();
    std::uint8_t get_version() const;
    std::uint32_t get_ihl() const;
    std::vector<std::uint8_t> get_bytes() const;

    friend std::istream& operator>>(std::istream& is, ipv4_header& header);
    friend std::ostream& operator<<(std::ostream& os, const ipv4_header& header);
};

class ipv4_header_builder
{
    ipv4_header m_header;
public:
    ipv4_header_builder& set_dscp(const std::uint8_t& dscp);
    ipv4_header_builder& set_ecn(const std::uint8_t& ecn);
    ipv4_header_builder& set_payload_length(const std::uint16_t& payload_length);
    ipv4_header_builder& set_identification(const std::uint16_t& identification_);
    ipv4_header_builder& set_flag_dont_fragment(bool dont_fragment);
    ipv4_header_builder& set_ttl(const std::uint8_t ttl);
    ipv4_header_builder& set_protocol(const std::uint8_t& protocol);
    ipv4_header_builder& set_source_address(const std::uint32_t& src_addr);
    ipv4_header_builder& set_destination_address(const std::uint32_t& dst_addr);
    ipv4_header_builder& set_options(const ipv4_header::OptionsVector& opt);
    ipv4_header finalize_build();
};
}