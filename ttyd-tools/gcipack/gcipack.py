import ctypes
import math
import os
import struct
import sys
from datetime import datetime, timezone
from pathlib import Path

# Check arguments
input_filename = sys.argv[1]
with open(input_filename, 'rb') as input_file:
    input_buffer = ctypes.create_string_buffer(input_file.read())[:-1]


# Load banner
with open(sys.argv[5], 'rb') as banner_file:
    banner_buffer = ctypes.create_string_buffer(banner_file.read())[:-1]
    if len(banner_buffer) != 0x1800:
        print('Warning: banner size mismatch (should be 96x32 RGB5A3)')


# Load icon
with open(sys.argv[6], 'rb') as icon_file:
    icon_buffer = ctypes.create_string_buffer(icon_file.read())[:-1]
    if len(icon_buffer) != 0x800:
        print(
            f'Warning: icon size mismatch {len(icon_buffer)} (should be 32x32 RGB5A3)',
        )


# Comment
comment_buffer = ctypes.create_string_buffer(0x40)
struct.pack_into('32s', comment_buffer, 0x00, sys.argv[3].encode())
struct.pack_into('32s', comment_buffer, 0x20, sys.argv[4].encode())


# File info
file_info_buffer = ctypes.create_string_buffer(0x200 - 0x40)
struct.pack_into('>L', file_info_buffer, 0, len(input_buffer))


# Pad to block boundary
file_length = (
    len(banner_buffer)
    + len(icon_buffer)
    + len(comment_buffer)
    + len(file_info_buffer)
    + len(input_buffer)
)
block_count = math.ceil(file_length / 0x2000)
padding_length = block_count * 0x2000 - file_length
padding_buffer = ctypes.create_string_buffer(padding_length)


# Create header
header_buffer = ctypes.create_string_buffer(0x40)
struct.pack_into('4s', header_buffer, 0x00, sys.argv[7].encode())  # game code
struct.pack_into('>H', header_buffer, 0x04, 0x3031)  # maker code
struct.pack_into('>B', header_buffer, 0x06, 0xFF)  # unused
struct.pack_into('>B', header_buffer, 0x07, 2)  # banner flags (RGB5A3)
struct.pack_into('32s', header_buffer, 0x08, sys.argv[2].encode())  # filename
struct.pack_into(
    '>L', header_buffer, 0x28, int((
        datetime.now(timezone.utc) - datetime(2000, 1, 1, tzinfo=timezone.utc)
    ).total_seconds()),
)  # modified time
struct.pack_into('>L', header_buffer, 0x2C, 0)  # image offset
struct.pack_into('>H', header_buffer, 0x30, 2)  # icon format
struct.pack_into('>H', header_buffer, 0x32, 3)  # animation speed (1 icon for 12 frames)
struct.pack_into('>B', header_buffer, 0x34, 4)  # permissions
struct.pack_into('>B', header_buffer, 0x35, 0)  # copy counter
struct.pack_into('>H', header_buffer, 0x36, 0)  # first block number
struct.pack_into('>H', header_buffer, 0x38, block_count)  # block count
struct.pack_into('>H', header_buffer, 0x3A, 0xFF)  # unused
struct.pack_into('>L', header_buffer, 0x3C, 0x2000)  # comment address


input_path = Path(input_filename)
output_filename = f'{input_path.stem}.gci'
with open(output_filename, 'wb') as output_file:
    output_file.write(bytearray(header_buffer))
    output_file.write(bytearray(banner_buffer))
    output_file.write(bytearray(icon_buffer))
    output_file.write(bytearray(comment_buffer))
    output_file.write(bytearray(file_info_buffer))
    output_file.write(bytearray(input_buffer))
    output_file.write(bytearray(padding_buffer))
