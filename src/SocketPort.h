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
 *   @file   SocketPort.h
 *
 *   @brief  Implements a TCP socket port.
 *
 ****************************************************************************/

#pragma once

#include <cinttypes>

#include "Port.h"

//! @addtogroup bus
//! @{

namespace bus {

//! @brief Abstract base class for a port used to talk to bioloid devices.
class SocketPort : public IPort {
 public:
    using Port = uint16_t;  //!< Type to hold a port number.

    //! Constructor.
    Socket(
        int socket  //!< [in] Connected Socket for connection.
    );

    //! Destructor.
    ~SocketPort() override;

    //! Returns true if data is available to be read.
    bool isDataAvailable() const override;

    //! Reads a byte from the port (or other virtual device).
    //! This function is non-blocking.
    //! @returns true if a byte was read, false otherwise.
    bool readByte(uint8_t* byte) override;

    //! Returns true if space is available to write a byte.
    bool isSpaceAvailable() const override;

    //! Writes a byte to the port.
    //! @returns true if the byte was written successfully.
    bool writeByte(uint8_t byte  //!< [in] byte to write.
                             ) override;

 private:
    int m_socket;  //!< Connected socket.
};

}  // namespace bus

//! @}
