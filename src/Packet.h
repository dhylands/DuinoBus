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
 *   @file   Packet.h
 *
 *   @brief  Parses packets.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>
#include <initializer_list>

#include "DuinoUtil.h"

//! Forward declaration.
//@{
class PacketDecodeTest_BadStateTest_Test;
class PacketEncodeTest_BadStateTest_Test;
//@}

namespace packet {

//! Error code.
enum class Error {
    NONE = 0,           //!< No Error.
    NOT_DONE = 1,       //!< Indicates that parsing is not complete.
    CRC = 2,            //!< CRC error occurred during parsing.
    TIMEOUT = 3,        //!< Indicates that a timeout occurred while waiting for a reply.
    TOO_MUCH_DATA = 4,  //!< Packet storage isn't big enough.
    TOO_SMALL = 5,      //!< Not enough data for a packet.
    BAD_STATE = 6,      //!< Not enough data for a packet.
};

//! @brief Predefined commands.
//! @details We use a struct rather than an enum so that a device can derive their own commands.
struct Command : Bits<uint8_t> {
    static constexpr Type PING = 0x01;        //!< Checks to see if the device is alive

    //! @brief Returns the string version of a command.
    //! @details By making this virtual, derived classes can add custom commands and also
    //!          return the string equivalents.
    //! @returns A pointer to a C string containing the string equivalent of the command.
    virtual const char* as_str() {
        switch (this->value) {
            case PING:
                return "PING";
        }
        return "???";
    }
};

//! @brief Encapsulates the packet sent to a devices.
//! @details The over the wire format looks like a SLIP encoded packet.
//!          Packets are SLIP encoded, and the length is inferred from the decoded packet.
//!          The first byte of each packet is the command.
//!          The last byte of the packet is an 8-bit CRC (crcmod.predefined.mkCrcFun('crc-8'))
//!          Each packet has data bytes between the command and the CRC.
class Packet {
 public:
    //! Default constructor
    Packet();

    //! Constructor where the storage for parameter data is specified.
    Packet(
        size_t maxData,  //!< [in] Maximum number of data bytes in the packet.
        void* data       //!< [in] Place to store packet data.
    );

    //! Destructor
    ~Packet();

    //! Returns the command from the command packet.
    //! @returns Command::Type containg the command found in the packet.
    Command::Type command() const { return this->m_data[0]; }

    //! Sets the command for the packet using a Command object.
    void command(Command cmd  //!< [in] Command object to set command from..
    ) {
        this->m_data[0] = cmd.value;
    }

    //! Sets the command for the packet using a CommandType.
    void command(Command::Type cmd  //!< [in] Command to set command to.
    ) {
        this->m_data[0] = cmd;
    }

    //! Returns the length of the data in the packet.
    //! The length doesn't include the command or CRC.
    //! @returns the number of bytes of data in the packet.
    uint8_t length() const { return this->m_dataLen - 2; }

    //! Returns a mutable pointer to the data portion of the packet.
    uint8_t* data() { return &this->m_data[1]; }

    //! Returns a const pointer to the data portion of the packet.
    uint8_t const* data() const { return &this->m_data[1]; }

    //! Sets the packet data.
    //! To set the packet to be empty, pass in a zero dataLen.
    void setData(
        size_t dataLen,   //!< [in] Size of the packet data to copy in.
        void const* data  //!< [in] Packet data to copy in.
    ) {
        this->m_dataLen = 1;  // For the command
        this->appendData(dataLen, data);
    }

    //! Appends data to the packet.
    void appendData(
        size_t dataLen,   //!< [in] Size of the packet data to copy in.
        void const* data  //!< [in] Packet data to copy in.
    );

    //! Returns the CRC contained in the packet.
    uint8_t crc() const {
        assert(this->m_dataLen >= 2);  // Command + CRC
        return this->m_data[this->m_dataLen -1];
    }

    //! Runs a single byte through the packet parser state machine.
    //! @returns Error::NONE if the packet was parsed successfully.
    //! @returns Error::NOT_DONE if the packet is incomplete.
    //! @returns Error::CHECKSUM if a checksum error was encountered.
    //! @returns Error::TOO_MUCH_DATA if the data doesn't fit into the packet data.
    Error processByte(uint8_t byte  //!< [in] Byte to parse.
    );

    //! Resets the encoder to start encoding a packet.
    void encodeStart();

    //! Encodes the next byte of the packet.
    //! @returns Error::NONE if the packet encoding has been completed.
    //! @returns Error::NOT_DONE if packet encoding is incomplete.
    Error encodeByte(
        uint8_t* byte  //!< [out] Place to store the next encoded byte.
    );

 private:
    //! This allows the TEST(PacketTest, BadState) function to access m_state
    friend class ::PacketDecodeTest_BadStateTest_Test;
    friend class ::PacketEncodeTest_BadStateTest_Test;

    enum class State {
        IDLE,     //!< Haven't started parsing a packet yet.
        PACKET,   //!< Parsing a packet
        ESCAPE,   //!< Parsing an escape
    };

    size_t const m_maxData = 0;       //!< Max number of bytes of packet data.
    size_t m_dataLen = 0;             //!< Length of the packet.
    uint8_t* const m_data = nullptr;  //!< Place to store packet data.
    State m_state = State::IDLE;      //!< State of the parser.
    size_t m_encodeIdx = 0;
};

}  // namespace packet
