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
 *   @file   Bus.h
 *
 *   @brief  Abstract base class for a bus.
 *
 ****************************************************************************/

#pragma once

#include "Packet.h"
#include "Port.h"

namespace bus {

class IBus {
 public:

    //! Reads a byte from the port, and runs it through the packet parser.
    packet::Error processByte();

    //! Returns a pointer to the most recently parsed packet.
    packet::Packet* getPacket(void) { return &this->m_rcvPacket; }

    packet::Error bus::IBus::writePacket(packet::Packet* packet);


 private:
    bus::IPort* m_port;
    packet::Packet m_rcvPacket;
};

}  // bus
