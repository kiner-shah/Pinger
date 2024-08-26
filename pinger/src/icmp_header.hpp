// Copyright (c) 2024, Kiner Shah
// This source code is distributed under BSD-3 Clause License
// details of which can be found in LICENSE file.

#pragma once

#include <vector>
#include <cstdint>
#include <ostream>
#include <istream>

namespace pinger
{

constexpr std::uint8_t ICMP_TYPE_ECHO_REPLY = 0;
constexpr std::uint8_t ICMP_TYPE_ECHO_REQUEST = 8;
constexpr std::uint8_t ICMP_TYPE_TIME_EXCEEDED = 11;

constexpr std::uint8_t ICMP_CODE_TIME_EXCEEDED_TTL_EXCEEDED = 0;
constexpr std::uint8_t ICMP_CODE_TIME_EXCEEDED_FRAG_REASSEM_TIME_EXCEEDED = 1;

struct alignas(4) icmp_header
{
    std::uint8_t type = 0;
    std::uint8_t code = 0;
    std::uint16_t checksum = 0;
    std::uint16_t identifier = 0;
    std::uint16_t sequence_number = 0;

    friend std::ostream& operator<<(std::ostream& os, const icmp_header& header);
    friend std::istream& operator>>(std::istream& is, icmp_header& header);
};

class icmp_header_builder
{
    icmp_header m_header;
    std::vector<std::uint8_t> m_payload;

    std::uint16_t compute_icmp_checksum();
public:
    icmp_header_builder& set_type(const std::uint8_t& type_);
    icmp_header_builder& set_code(const std::uint8_t& code_);
    icmp_header_builder& set_identifier(const std::uint16_t& id);
    icmp_header_builder& set_sequence_number(const std::uint16_t& seq_no);
    icmp_header_builder& set_payload(const std::vector<std::uint8_t>& payload);
    icmp_header finalize_build();
};

}   // namespace pinger
