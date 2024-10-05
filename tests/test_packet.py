#!/usr/bin/env python3

# This file tests the packet parser

import unittest
import binascii

from duino_bus.dump_mem import dump_mem
from duino_bus.packet import ErrorCode, Packet


class TestPacket(unittest.TestCase):

    def test_too_small(self):
        pass


if __name__ == '__main__':
    unittest.main()
