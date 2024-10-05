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
 *   @file   Packet.cpp
 *
 *   @brief  Parses bioloid packets.
 *
 ****************************************************************************/

#include "Packet.h"

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <memory>

#include "Crc8.h"
#include "DumpMem.h"
#include "Log.h"

//! @addtogroup bus
//! @{

namespace packet {

static constexpr uint8_t END = 0xC0;      //!< Start/End of Frame
static constexpr uint8_t ESC = 0xDB;      //!< Next char is escaped
static constexpr uint8_t ESC_END = 0xDC;  //!< Escape an END character
static constexpr uint8_t ESC_ESC = 0xDD;  //!< Escape an ESC character

//Packet::Packet() {}

Packet::Packet(size_t maxData, void* data)
    : m_maxData(maxData),
      m_data{reinterpret_cast<uint8_t*>(data)} {
}

Packet::~Packet() {}

void Packet::appendData(
    size_t dataLen,   //!< [in] Size of the packet data to copy in.
    void const* data  //!< [in] Packet data to copy in.
) {
    assert(this->m_dataLen + dataLen <= this->m_maxData);
    if (dataLen > 0) {
        memcpy(&this->m_data[this->m_dataLen], data, dataLen);
        this->m_dataLen += dataLen;
    }
}

Error Packet::processByte(uint8_t byte) {
    switch (this->m_state) {
        case State::IDLE: {  // We're waiting for the beginning of the packet (0xC0)
            this->m_state = State::PACKET;
            this->m_dataLen = 0;
            return Error::NOT_DONE;
        }

        case State::PACKET: {
            if (byte == END) {
                if (this->m_dataLen == 0) {
                    // We ignore empty packets.
                    return Error::NOT_DONE;
                }
                if (this->m_dataLen == 1) {
                    // Minimum packet requires a Cmd and CRC
                    return Error::TOO_SMALL;
                }
                DumpMem("Rcvd", 0, this->m_data, this->m_dataLen);
                uint8_t rcvd_crc = this->m_data[this->m_dataLen - 1];
                uint8_t expected_crc = Crc8(0, this->m_dataLen - 1, this->m_data);
                if (rcvd_crc == expected_crc) {
                    this->m_state = State::IDLE;
                    return Error::NONE;
                }
                Log::error("CRC Error: Received 0x%02" PRIx8 " Expected 0x%02" PRIx8, rcvd_crc, expected_crc);
                return Error::CRC;
                }
            if (this->m_dataLen >= this->m_maxData) {
                return Error::TOO_MUCH_DATA;
            }
            if (byte == ESC) {
                this->m_state = State::ESCAPE;
                return Error::NOT_DONE;
            }
            this->m_data[this->m_dataLen++] = byte;
            return Error::NOT_DONE;
        }

        case State::ESCAPE: {
            if (byte == ESC_END) {
                this->m_data[this->m_dataLen++] = END;
            } else if (byte == ESC_ESC) {
                this->m_data[this->m_dataLen++] = ESC;
            } else {
                this->m_data[this->m_dataLen++] = byte;
            }
            this->m_state = State::PACKET;
            return Error::NOT_DONE;
        }
    }
    return Error::BAD_STATE;
}

void Packet::encodeStart() {
    this->m_state = State::IDLE;
}

Error Packet::encodeByte(uint8_t* byte) {
    switch (this->m_state) {
        case State::IDLE: {
            *byte = END;
            this->m_encodeIdx = 0;
            this->m_state = State::PACKET;
            return Error::NOT_DONE;
        }

        case State::PACKET: {
            if (this->m_encodeIdx >= this->m_dataLen) {
                if (this->m_encodeIdx == this->m_dataLen) {
                    *byte = Crc8(0, this->m_dataLen, this->m_data);
                    this->m_encodeIdx++;
                    return Error::NOT_DONE;
                }
                *byte = END;
                this->m_state = State::IDLE;
                return Error::NONE;
            }
            uint8_t nextByte = this->m_data[this->m_encodeIdx];
            if (nextByte == END || nextByte == ESC) {
                *byte = ESC;
                this->m_state = State::ESCAPE;
                return Error::NOT_DONE;
            }

            *byte = nextByte;
            this->m_encodeIdx++;
            return Error::NOT_DONE;
        }

        case State::ESCAPE: {
            uint8_t nextByte = this->m_data[this->m_encodeIdx];
            if (nextByte == END) {
                nextByte = ESC_END;
            } else {
                nextByte = ESC_ESC;
            }
            *byte = nextByte;
            this->m_encodeIdx++;
            this->m_state = State::PACKET;
            return Error::NOT_DONE;
        }
    }
    return Error::BAD_STATE;
}

}  // namespace packet

//! @}  bus group
