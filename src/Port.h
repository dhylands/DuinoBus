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
 *   @file   Port.h
 *
 *   @brief  Base class for implementing a port.
 *
 ****************************************************************************/

#pragma once

#include <cinttypes>

//! @addtogroup bus
//! @{

namespace bus {

//! @brief Abstract base class for a port used to talk to bioloid devices.
class IPort {
 public:
    //! @brief Destructor.
    virtual ~IPort() = default;

    //! Returns true if data is available to be read.
    virtual bool isDataAvailable() const = 0;

    //! Reads a byte from the port (or other virtual device).
    //! This function is non-blocking.
    //! @returns true if a byte was read, false otherwise.
    virtual bool readByte(uint8_t* byte) = 0;

    //! Returns true if space is available to write a byte.
    virtual bool isSpaceAvailable() const = 0;

    //! Writes a byte to the port.
    virtual void writeByte(uint8_t byte  //!< [in] byte to write.
                             ) = 0;
};

}  // namespace bus

//! @}
