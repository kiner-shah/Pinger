// Copyright (c) 2024, Kiner Shah
// This source code is distributed under BSD-3 Clause License
// details of which can be found in LICENSE file.

#include "ipv4_header.hpp"
#include "utils.hpp"
#include <algorithm>

namespace pinger
{
std::uint8_t ipv4_header::get_version() const
{
    return (version_and_ihl >> 4) & 0b00001111;
}

std::uint32_t ipv4_header::get_ihl() const
{
    return (version_and_ihl & 0b00001111) * 4;
}

std::vector<std::uint8_t> ipv4_header::get_bytes() const
{
    std::vector<std::uint8_t> buffer(IPV4_HEADER_MAX_TOTAL_SIZE, 0);
    std::fill_n(buffer.begin(), buffer.size(), 0);
    std::copy_n(reinterpret_cast<char*>(const_cast<ipv4_header*>(this)), IPV4_HEADER_SIZE_EXCLUDING_OPTIONS_IN_BYTES, buffer.begin());
    const std::uint8_t options_length = get_ihl() - IPV4_HEADER_SIZE_EXCLUDING_OPTIONS_IN_BYTES;
    buffer.resize(IPV4_HEADER_SIZE_EXCLUDING_OPTIONS_IN_BYTES + options_length);
    std::copy_n(this->options.begin(), options_length, buffer.begin() + IPV4_HEADER_SIZE_EXCLUDING_OPTIONS_IN_BYTES);
    return buffer;
}

void ipv4_header::compute_and_set_header_checksum()
{
    std::uint32_t sum = 0;
    // We need to compute sum of 16-bit values, so need to pack both in one 16-bit variable
    sum += (version_and_ihl << 8) + dscp_and_ecn;
    sum += total_length + identification + flags_and_fragment_offset;
    sum += (time_to_live << 8) + protocol;
    sum += source_address;
    sum += destination_address;

    for (std::size_t i = 0; i < options.size(); i += 2)
    {
        sum += (options.at(i) << 8);
        if (i + 1 < options.size())
        {
            sum += options.at(i + 1);
        }
    }
    // Now sum is 32-bit, so we need to add top 16-bits to bottom 16-bits in order
    // to have a 16-bit value;
    std::uint32_t temp_sum = (sum >> 16);
    temp_sum += static_cast<std::uint16_t>(sum);

    // Due to above addition, maybe some carry was introduced in top 16-bits, need to
    // handle add the carry to bottom 16-bits
    temp_sum += (temp_sum >> 16);

    // Now we strip off the top 16-bits, leaving behind bottom 16-bits
    // And we perform 1s complement operation on the bottom 16-bits
    header_checksum = ~static_cast<std::uint16_t>(temp_sum);
}

std::ostream& operator<<(std::ostream& os, const ipv4_header& header)
{
    std::uint16_t total_length = host_to_network_short(header.total_length);
    std::uint16_t identification = host_to_network_short(header.identification);
    std::uint16_t flags_and_fragment_offset = host_to_network_short(header.flags_and_fragment_offset);
    std::uint16_t header_checksum = host_to_network_short(header.header_checksum);

    os << header.version_and_ihl << header.dscp_and_ecn;
    os.write(reinterpret_cast<const char*>(&total_length), sizeof(total_length));
    os.write(reinterpret_cast<const char*>(&identification), sizeof(identification));
    os.write(reinterpret_cast<const char*>(&flags_and_fragment_offset), sizeof(flags_and_fragment_offset));
    os << header.time_to_live << header.protocol;
    os.write(reinterpret_cast<const char*>(&header_checksum), sizeof(header_checksum));
    os.write(reinterpret_cast<const char*>(&header.source_address), sizeof(header.source_address));
    os.write(reinterpret_cast<const char*>(&header.destination_address), sizeof(header.destination_address));

    auto options_length = header.get_ihl() - IPV4_HEADER_SIZE_EXCLUDING_OPTIONS_IN_BYTES;
    for (std::size_t i = 0; i < options_length; i++)
    {
        os << header.options.at(i);
    }
    return os;
}

std::istream& operator>>(std::istream& is, ipv4_header& header)
{
    std::array<char, IPV4_HEADER_MAX_TOTAL_SIZE> buffer;
    if (!is.read(buffer.data(), IPV4_HEADER_SIZE_EXCLUDING_OPTIONS_IN_BYTES))
    {
        return is;
    }
    header.version_and_ihl = buffer[0];
    if (header.get_version() != 4)
    {
        is.setstate(std::ios::failbit);
        return is;
    }
    auto options_length = header.get_ihl() - IPV4_HEADER_SIZE_EXCLUDING_OPTIONS_IN_BYTES;
    if (options_length < 0 || options_length > IPV4_HEADER_OPTIONS_MAX_SIZE_IN_BYTES)
    {
        is.setstate(std::ios::failbit);
    }
    else
    {
        is.read(buffer.data() + IPV4_HEADER_SIZE_EXCLUDING_OPTIONS_IN_BYTES, options_length);
    }

    header.dscp_and_ecn = buffer[1];
    header.total_length = (buffer[2] << 8) | buffer[3];
    header.identification = (buffer[4] << 8) | buffer[5];
    header.flags_and_fragment_offset = (buffer[6] << 8) | buffer[7];
    header.time_to_live = buffer[8];
    header.protocol = buffer[9];
    header.header_checksum = (buffer[10] << 8) | buffer[11];
    std::copy_n(buffer.data() + 12, 4, &header.source_address);
    std::copy_n(buffer.data() + 16, 4, &header.destination_address);
    std::copy_n(buffer.data() + 20, options_length, header.options.begin());

    return is;
}

ipv4_header_builder &ipv4_header_builder::set_dscp(const std::uint8_t &dscp)
{
    m_header.dscp_and_ecn = (m_header.dscp_and_ecn & 0b00000011) | (dscp << 2);
    return *this;
}

ipv4_header_builder& ipv4_header_builder::set_ecn(const std::uint8_t &ecn)
{
    m_header.dscp_and_ecn = (m_header.dscp_and_ecn & 0b11111100) | ecn;
    return *this;
}

ipv4_header_builder& ipv4_header_builder::set_payload_length(const std::uint16_t &payload_length)
{
    m_header.total_length = m_header.get_ihl() + payload_length;
    return *this;
}

ipv4_header_builder &ipv4_header_builder::set_identification(const std::uint16_t &identification_)
{
    m_header.identification = identification_;
    return *this;
}

ipv4_header_builder &ipv4_header_builder::set_flag_dont_fragment(bool dont_fragment)
{
    m_header.flags_and_fragment_offset = (m_header.flags_and_fragment_offset & 0xbfff) | (dont_fragment ? (1 << 14) : 0);
    return *this;
}

ipv4_header_builder &ipv4_header_builder::set_ttl(const std::uint8_t ttl)
{
    m_header.time_to_live = ttl;
    return *this;
}

ipv4_header_builder &ipv4_header_builder::set_protocol(const std::uint8_t &protocol)
{
    m_header.protocol = protocol;
    return *this;
}

ipv4_header_builder &ipv4_header_builder::set_source_address(const std::uint32_t &src_addr)
{
    m_header.source_address = src_addr;
    return *this;
}

ipv4_header_builder &ipv4_header_builder::set_destination_address(const std::uint32_t &dst_addr)
{
    m_header.destination_address = dst_addr;
    return *this;
}

ipv4_header_builder &ipv4_header_builder::set_options(const ipv4_header::OptionsVector &opt)
{
    if (opt.empty())
    {
        return *this;
    }
    if (opt.size() * 8 > IPV4_HEADER_OPTIONS_MAX_SIZE_IN_BYTES)
    {
        // TODO: should we set partial options vector?
        return *this;
    }
    std::fill(m_header.options.begin(), m_header.options.end(), 0);
    std::copy_n(opt.begin(), opt.size(), m_header.options.begin());
    std::uint8_t no_of_32_bit_words = (opt.size() % 4 == 0 ? opt.size() / 4 : opt.size() / 4 + 1);
    m_header.version_and_ihl = (m_header.version_and_ihl & 0b11110000) | no_of_32_bit_words;
    return *this;
}

ipv4_header ipv4_header_builder::finalize_build()
{
    m_header.compute_and_set_header_checksum();
    return m_header;
}

}   // namespace pinger