"""
This module defines Packet class.
"""

from typing import ByteString, Union
import crcmod

from duino_bus.dump_mem import dump_mem


# pylint: disable=too-few-public-methods
class ErrorCode:
    """Constants for Errors returns by the parsing funtions."""
    NONE = 0  # No Error
    NOT_DONE = 1  # Indicates that parsing is not complete
    CRC = 2  # CRC error occurred during parsing
    TIMEOUT = 3  # Indicates that a timeout occurred while waiting for a reply.
    TOO_MUCH_DATA = 4  # Packet storage isn't big enough
    TOO_SMALL = 5  # Not enough data for a packet
    BAD_STATE = 6  # Bad parsing state
    OS = 7  # OS error


class Packet:
    """
    Encapsulates the packets sent between devices.
    Packets are SLIP encoded, and the length is inferred from the decoded packet.
    The first byte of each packet is the command.
    The last byte of the packet is an 8-bit CRC (crcmod.predefined.mkCrcFun('crc-8'))
    Each packet has data bytes between the command and the CRC.
    """

    END = 0xC0  # Start/End of Frame
    ESC = 0xDB  # Next char is escaped
    ESC_END = 0xDC  # Escape an END character
    ESC_ESC = 0xDD  # Escape an ESC character

    STATE_IDLE = 0  # Haven't started parsing a packet yet.
    STATE_PACKET = 1  # Parsing a packet
    STATE_ESCAPE = 2  # Parsing an escape

    MAX_DATA_LEN = 256

    CRC_FN = crcmod.predefined.mkCrcFun('crc-8')

    def __init__(self, cmd: Union[int, None] = None, data: Union[ByteString, None] = None) -> None:
        """Constructs a packet from a buffer, if provided."""
        self.cmd = cmd
        if data is None:
            self.data = bytearray()
        else:
            self.data = data
        self.crc = 0

    def dump(self, label: str) -> None:
        """
        Dumps the contents of a packet.
        """
        print(
                f'{label} Command: 0x{self.cmd:02x} ({str(self.cmd)}) Len: {len(self.data)} '
                f'CRC: 0x{self.crc:02x}'
        )
        if len(self.data) > 0:
            dump_mem(self.data, label)

    def get_command(self) -> int:
        """Returns the command from the packet."""
        return self.cmd

    def set_command(self, cmd: int) -> None:
        """Sets the command in the packet."""
        self.cmd = cmd

    def get_data_len(self) -> int:
        """Returns the length of the data portion of the packet."""
        return len(self.data)

    def get_data(self) -> bytearray:
        """Returns the data portion of the packet."""
        return self.data

    def get_data_byte(self, idx: int) -> int:
        """Returns one byte of the packet data."""
        return self.data[idx]

    def set_data(self, data: bytearray) -> None:
        """Sets the packet data."""
        self.data = data

    def append_byte(self, byte: int) -> None:
        """Appends a byte to the packet data."""
        self.data.append(byte)

    def append_data(self, data: bytearray) -> None:
        """Appends data to the packet data."""
        self.data.extend(data)

    def get_crc(self) -> int:
        """Returns the CRC from the packet."""
        return self.crc

    def calc_crc(self) -> int:
        """Calculates and returns the CRC using the command/packet data."""
        crc = Packet.CRC_FN(self.cmd.to_bytes(1, 'little'), 0)
        return Packet.CRC_FN(self.data, crc)

    def calc_and_store_crc(self) -> None:
        """Calculates the CRC of the data and saves it in the packet."""
        self.crc = self.calc_crc()

    def extract_crc(self) -> int:
        """Used by the packet decoder, this function extracts the CRC from the
           end of the data and stores it in the CRC.
        """
        self.crc = self.data[-1]
        self.data = self.data[:-1]
        return self.crc


#    ######################################################################
#
#    def write_packet(self, write_fn: Callable) -> None:
#        """
#        Writes a packet sending each byte thru `write_fn()`
#        """
#        self.write_raw_byte(Packet.END, write_fn)
#        self.write_escaped_byte(self.cmd, write_fn)
#        crc = self.crc(bytes([self.cmd]))
#        if len(self.data) > 0:
#            for byte in self.data:
#                self.write_escaped_byte(byte, write_fn)
#            crc = self.crc(self.data, crc)
#        self.write_escaped_byte(crc, write_fn)
#        self.write_raw_byte(Packet.END, write_fn)
#
#    def write_escaped_byte(self, byte: int, write_fn: Callable) -> None:
#        """
#        Writes a byte thru `write_fn()` escaping as necessary.
#        """
#        if byte == Packet.END:
#            self.write_raw_byte(Packet.ESC, write_fn)
#            self.write_raw_byte(Packet.ESC_END, write_fn)
#        elif byte == Packet.ESC:
#            self.write_raw_byte(Packet.ESC, write_fn)
#            self.write_raw_byte(Packet.ESC_ESC, write_fn)
#        else:
#            self.write_raw_byte(byte, write_fn)
#
#    def write_raw_byte(self, byte: int, write_fn: Callable) -> None:
#        """
#        Writes a byte thru `write_fn()` with no escaping.
#        """
#        write_fn(byte)
#
#    def parse_byte(self, byte: int) -> int:
#        """
#        Runs a single byte through the packet parsing state machine.
#
#        Returns Error.NOT_DONE if the packet is incomplete,
#        Error.NONE if the packet was received successfully, and
#        Error.CRC if a checksum error is detected.
#        """
#        if self.state == Packet.STATE_IDLE:
#            return self.parse_byte_state_idle(byte)
#        if self.state == Packet.STATE_PACKET:
#            return self.parse_byte_state_packet(byte)
#        if self.state == Packet.STATE_ESCAPE:
#            return self.parse_byte_state_escape(byte)
#        return self.parse_byte_state_invalid(byte)
#
#    def parse_byte_state_idle(self, byte: int) -> int:
#        """
#        Runs a single byte through the packet parsing state IDLE.
#        """
#        if byte == Packet.END:
#            self.state = Packet.STATE_PACKET
#            self.data = bytearray()
#        return Error.NOT_DONE
#
#    def parse_byte_state_packet(self, byte: int) -> int:
#        """
#        Runs a single byte through the packet parsing state PACKET.
#        """
#        if byte == Packet.END:
#            if len(self.data) == 0:
#                # We ignore empty packets
#                return Error.NOT_DONE
#            if len(self.data) == 1:
#                # Mimimum packet requires a Cmd and a CRC
#                return Error.TOO_SMALL
#            dump_mem(self.data[:-1], 'CRC')
#            rcvd_crc = self.data[-1]
#            calc_crc = self.crc(self.data[:-1])
#            if rcvd_crc == calc_crc:
#                self.cmd = self.data[0]
#                self.data = self.data[1:-1]  # Strip off cmd and CRC
#                self.state = Packet.STATE_IDLE
#                return Error.NONE
#            print(f'CRC Error: Received 0x{rcvd_crc:02x} Expected: 0x{calc_crc:02x}')
#            return Error.CRC
#        if len(self.data) >= Packet.MAX_DATA_LEN:
#            self.state = Packet.STATE_IDLE
#            return Error.TOO_MUCH_DATA
#        if byte == Packet.ESC:
#            self.state = Packet.STATE_ESCAPE
#            return Error.NOT_DONE
#        self.data.append(byte)
#        return Error.NOT_DONE
#
#    def parse_byte_state_escape(self, byte: int) -> int:
#        """
#        Runs a single byte through the packet parsing state ESCAPE.
#        """
#        if byte == Packet.ESC_END:
#            self.data.append(Packet.END)
#        elif byte == Packet.ESC_ESC:
#            self.data.append(Packet.ESC)
#        else:
#            self.data.append(byte)
#        self.state = Packet.STATE_PACKET
#        return Error.NOT_DONE
#
#    def parse_byte_state_invalid(self, _: int) -> int:
#        """
#        Runs a single byte through the packet parsing state INVALID.
#        """
#        self.state = Packet.STATE_IDLE
#        return Error.BAD_STATE
