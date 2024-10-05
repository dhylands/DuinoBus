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
#include "Packet.h"
#include "Util.h"

using Command = packet::Command;
using Error = packet::Error;
using Packet = packet::Packet;

class PacketDecodeTest {
 public:
    explicit PacketDecodeTest(char const* str): m_data(AsciiHexToBinary(str)),
    m_packet{LEN(this->m_packetData), this->m_packetData} {}

    //! Parses all of the bytes from m_dataStream using the packet parser.
    //! @returns Error::NONE if a packet was parsed successfully.
    //! @returns Error::NOT_DONE if more bytes are needed to complete parsing the packet.
    //! @returns Error::TOO_MUCH_DATA if the packet has more parameters than we have storage for.
    //! @returns Error::CHECKSUM if a checksum error was encountered while parsing.
    Error::Type parseData() {
        for (uint8_t byte : this->m_data) {
            if (auto err = this->m_packet.processByte(byte); err != Error::NOT_DONE) {
                return err;
            }
        }
        return Error::NOT_DONE;
    }

    ByteBuffer m_data;
    Packet m_packet;
    uint8_t m_packetData[16];
};

TEST(PacketDecodeTest, EmptyPacketTest) {
    auto test = PacketDecodeTest("c0 c0");

    EXPECT_EQ(test.parseData(), Error::NOT_DONE);
}

TEST(PacketDecodeTest, NoDataTest) {
    auto test = PacketDecodeTest("c0 01 07 c0");

    EXPECT_EQ(test.parseData(), Error::NONE);
    EXPECT_EQ(test.m_packet.command(), Command::PING);
}

TEST(PacketDecodeTest, CrcErrorTest) {
    auto test = PacketDecodeTest("c0 01 08 c0");

    EXPECT_EQ(test.parseData(), Error::CRC);}

TEST(PacketDecodeTest, TooSmallTest) {
    auto test = PacketDecodeTest("c0 01 c0");

    EXPECT_EQ(test.parseData(), Error::TOO_SMALL);
}

TEST(PacketDecodeTest, OneByteDataTest) {
    auto test = PacketDecodeTest("c0 01 02 1b c0");

    EXPECT_EQ(test.parseData(), Error::NONE);
    EXPECT_EQ(test.m_packet.command(), Command::PING);
    EXPECT_EQ(test.m_packet.length(), 1);
    EXPECT_EQ(test.m_packet.data()[0], 0x02);
    EXPECT_EQ(test.m_packet.crc(), 0x1b);
}

TEST(PacketDecodeTest, TwoBytesDataTest) {
    auto test = PacketDecodeTest("c0 01 02 03 48 c0");

    EXPECT_EQ(test.parseData(), Error::NONE);
    EXPECT_EQ(test.m_packet.command(), Command::PING);
    EXPECT_EQ(test.m_packet.length(), 2);
    EXPECT_EQ(test.m_packet.data()[0], 0x02);
    EXPECT_EQ(test.m_packet.data()[1], 0x03);
    EXPECT_EQ(test.m_packet.crc(), 0x48);
}

TEST(PacketDecodeTest, EscapeEndTest) {
    auto test = PacketDecodeTest("c0 db dc 02 03 ae c0");

    EXPECT_EQ(test.parseData(), Error::NONE);
    EXPECT_EQ(test.m_packet.command(), 0xc0);
    EXPECT_EQ(test.m_packet.length(), 2);
    EXPECT_EQ(test.m_packet.data()[0], 0x02);
    EXPECT_EQ(test.m_packet.data()[1], 0x03);
    EXPECT_EQ(test.m_packet.crc(), 0xae);
}

TEST(PacketDecodeTest, EscapeEscTest) {
    auto test = PacketDecodeTest("c0 db dd 02 03 e0 c0");

    EXPECT_EQ(test.parseData(), Error::NONE);
    EXPECT_EQ(test.m_packet.command(), 0xdb);
    EXPECT_EQ(test.m_packet.length(), 2);
    EXPECT_EQ(test.m_packet.data()[0], 0x02);
    EXPECT_EQ(test.m_packet.data()[1], 0x03);
    EXPECT_EQ(test.m_packet.crc(), 0xe0);
}

TEST(PacketDecodeTest, EscapeOtherTest) {
    auto test = PacketDecodeTest("c0 db 01 02 03 48 c0");

    EXPECT_EQ(test.parseData(), Error::NONE);
    EXPECT_EQ(test.m_packet.command(), Command::PING);
    EXPECT_EQ(test.m_packet.length(), 2);
    EXPECT_EQ(test.m_packet.data()[0], 0x02);
    EXPECT_EQ(test.m_packet.data()[1], 0x03);
    EXPECT_EQ(test.m_packet.crc(), 0x48);
}

TEST(PacketDecodeTest, FullDataTest) {
    auto test = PacketDecodeTest("c0 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 14 c0");

    EXPECT_EQ(test.parseData(), Error::NONE);
}

TEST(PacketDecodeTest, TooMuchDataTest) {
    auto test = PacketDecodeTest("c0 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f e0 c0");

    EXPECT_EQ(test.parseData(), Error::TOO_MUCH_DATA);
}

TEST(PacketDecodeTest, BadStateTest) {
    auto test = PacketDecodeTest("c0 00 01 07 c0");
    test.m_packet.m_state = static_cast<Packet::State>(0x80);
    EXPECT_EQ(test.parseData(), Error::BAD_STATE);
}

TEST(PacketDecodeDeathTest, CrcNoDataTest) {
    auto test = PacketDecodeTest("c0");
    ASSERT_DEATH(
        { test.m_packet.crc(); }, "Assertion `this->m_dataLen >= 2' failed.");
}
