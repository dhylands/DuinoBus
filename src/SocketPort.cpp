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
 *   @file   SocketPort.cpp
 *
 *   @brief  Implements a TCP socket port.
 *
 ****************************************************************************/

#include "SocketPort.h"

#include <fcntl.h>
#include <unistd.h>

bus::SocketPort::SocketPort(int socket) : m_socket(socket) {
    // Try to make the socket non-blocking.
    int flags = ::fcntl(this->m_socket, F_GETFL, 0);
    if (flags >= 0) {
        flags |= O_NONBLOCK;
        ::fcntl(this->m_socket, F_SETFL, flags);
    }
}

bus::SocketPort::~SocketPort() {
    ::close(this->m_socket);
}

bool bus::SocketPort::isDataAvailable() const {
    struct pollfd {
        .fd = this->m_socket,
        .events = POLLIN,
        .revents = 0;
    } pfd;
    
    return poll(&pfd, 1, 0) > 0;
}

bool bus::SocketPort::readByte(uint8_t* byte) {
    return ::recv(this->m_socket, byte, 1) == 1;
}

bool bus::SocketPort::isSpaceAvailable() const {
    struct pollfd {
        .fd = this->m_socket,
        .events = POLLOUT,
        .revents = 0;
    } pfd;
    
    return poll(&pfd, 1, 0) > 0;
}

bool bus::SocketPort::writeByte(uint8_t byte) {
    return ::send(this->m_socket, &byte, 1) == 1;
}
