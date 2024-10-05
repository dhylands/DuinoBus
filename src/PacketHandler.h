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
 *   @file   PacketHandler.h
 *
 *   @brief  Allows modular packet handlers to be implemented.
 *
 ****************************************************************************/

#pragma once

#include "Bus.h"
#include "Packet.h"

//! An abstract base class for implementing packet handlers.
class IPacketHandler {
 public:
    //! Function called to handle an incoming packet.
    //! @returns true if the packet was handled, false if it wasn't.
    virtual bool handlePacket(
        Packet const& cmd,  //!< [in] Packet that was received.
        Packet* rsp         //!< [out] Place to store response.
        ) = 0;
};
