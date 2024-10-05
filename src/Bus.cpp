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
 *   @file   Bus.cpp
 *
 *   @brief  Abstract base class for a bus.
 *
 ****************************************************************************/

#include "Bus.h"

packet::Error bus::IBus::processByte() {
    uint8_t byte;
    if (!this->m_port->readByte(&byte)) {
        return packet::Error::NOT_DONE;
    }
    return this->m_rcvPacket.processByte(byte);
}

packet::Error bus::IBus::writePacket(packet::Packet* packet) {
    packet->encodeStart();
    uint8_t byte;
    packet::Error err;
    while ((err = packet->encodeByte(&byte)) == packet::Error::NOT_DONE) {
        this->m_port->writeByte(byte);
    }
}