/****************************************************************************
 *
 *   @copyright Copyright (c) 2024 Dave Hylands     <dhylands@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the MIT License version as described in the
 *   LICENSE file in the root of this repository.
 *
 ****************************************************************************/
/**
 *   @file   Crc8Test.cpp
 *
 *   @brief  Tests for functions in Crc8.cpp
 *
 ****************************************************************************/

#include <gtest/gtest.h>

#include "AsciiHex.h"
#include "Crc8.h"
#include "DumpMem.h"
#include "LinuxColorLog.h"
#include "Packet.h"
#include "Util.h"

using Command = packet::Command;
using Error = packet::Error;
using Packet = packet::Packet;

class PacketEncodeTest {
 public:
    explicit PacketEncodeTest(Command::Type cmd, char const* str): m_data(AsciiHexToBinary(str)),
    m_packet{LEN(this->m_packetData), this->m_packetData} {
        debug();
        this->m_packet.command(cmd);
        this->m_packet.setData(this->m_data.size(), this->m_data.data());
        this->m_encodedData.clear();
        this->m_packet.encodeStart();
        uint8_t nextByte;
        while (this->m_packet.encodeByte(&nextByte) == Error::NOT_DONE) {
            m_encodedData.push_back(nextByte);
        }
        m_encodedData.push_back(nextByte);
    }

    bool matches(char const* expectedAsciiHexData) {
        auto expectedData = AsciiHexToBinary(expectedAsciiHexData);
        if (expectedData != this->m_encodedData) {
            DumpMem("Expecting", 0, expectedData.data(), expectedData.size());
            DumpMem("  Encoded", 0, this->m_encodedData.data(), this->m_encodedData.size());
        }
        return expectedData == this->m_encodedData;
    }

    ByteBuffer m_data;
    ByteBuffer m_encodedData;
    Packet m_packet;
    uint8_t m_packetData[16];
};

TEST(PacketEncodeTest, NoData) {
    auto test = PacketEncodeTest(Command::PING,  "");
    EXPECT_TRUE(test.matches("c0 01 07 c0"));
}

TEST(PacketEncodeTest, OneByteDataTest) {
    auto test = PacketEncodeTest(Command::PING, "02");
    EXPECT_TRUE(test.matches("c0 01 02 1b c0"));
}

TEST(PacketEncodeTest, TwoBytesDataTest) {
    auto test = PacketEncodeTest(Command::PING, "02 03");
    EXPECT_TRUE(test.matches("c0 01 02 03 48 c0"));
}

TEST(PacketEncodeTest, EscapeEndTest) {
    auto test = PacketEncodeTest(0xc0, "02 03");
    EXPECT_TRUE(test.matches("c0 db dc 02 03 ae c0"));
}

TEST(PacketEncodeTest, EscapeEscTest) {
    auto test = PacketEncodeTest(0xdb, "02 03");
    EXPECT_TRUE(test.matches("c0 db dd 02 03 e0 c0"));
}

TEST(PacketEncodeTest, BadStateTest) {
    auto test = PacketEncodeTest(Command::PING, "");

    uint8_t nextByte;
    test.m_packet.encodeStart();
    test.m_packet.m_state = static_cast<Packet::State>(0x80);
    EXPECT_EQ(test.m_packet.encodeByte(&nextByte), Error::BAD_STATE);
}

TEST(PacketEncodeDeathTest, AppdDataFullTest) {
    uint8_t packetData[4];
    Packet packet(LEN(packetData), packetData);

    uint8_t data[] = {1, 2, 3, 4, 5};

    packet.setData(0, nullptr);

    ASSERT_DEATH(
        { packet.appendData(LEN(data), data); }, "Assertion `this->m_dataLen \\+ dataLen <= this->m_maxData' failed.");
}
