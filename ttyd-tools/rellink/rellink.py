import ctypes
import os
import struct
import sys

# Read the input file
filename_in = sys.argv[1]
with open(filename_in, 'rb') as input_file:
    file_data = ctypes.create_string_buffer(input_file.read())


base_address = int(sys.argv[2], 16)  # 0x805ba9a0 # 0x804FFF3C


cur_offset = 0x0
rel_id = struct.unpack('>L', file_data[cur_offset : cur_offset + 0x4])[0]
cur_offset = 0xC
section_count = struct.unpack('>L', file_data[cur_offset : cur_offset + 0x4])[0]
cur_offset = 0x10
section_info_offset = struct.unpack('>L', file_data[cur_offset : cur_offset + 0x4])[0]
cur_offset = 0x28
import_offset = struct.unpack('>L', file_data[cur_offset : cur_offset + 0x4])[0]
cur_offset = 0x2C
import_size = struct.unpack('>L', file_data[cur_offset : cur_offset + 0x4])[0]

# set imports and relocations to invalid since we are doing them
struct.pack_into('>L', file_data, 0x24, 0)  # reloc offset
struct.pack_into('>L', file_data, 0x28, 0)  # import offset
struct.pack_into('>L', file_data, 0x2C, 0)  # import count

sections = []  # [list of [offset, size]]
imports = []  # [list of [id, offset]]

for i in range(section_count):
    cur_offset = section_info_offset + 8 * i

    _offset = (
        (struct.unpack('>L', file_data[cur_offset : cur_offset + 0x4])[0]) & ~1
    )  # remove bit 0 (exec bit)
    size = struct.unpack('>L', file_data[cur_offset + 0x4 : cur_offset + 0x8])[0]

    sections.append([_offset, size])

print(f'{section_count!s} sections')

print(sections)

for i in range(import_size // 8):
    cur_offset = import_offset + 8 * i

    _id = struct.unpack('>L', file_data[cur_offset : cur_offset + 0x4])[0]
    _offset = struct.unpack('>L', file_data[cur_offset + 0x4 : cur_offset + 0x8])[0]

    imports.append([_id, _offset])

print(f'{import_size / 8!s} import lists')

print(imports)


for import_entry in imports:
    cur_offset = import_entry[1]  # offset
    cur_rel_offset = 0
    cur_rel_section = 0
    while 1:
        cur_rel_offset += struct.unpack('>H', file_data[cur_offset : cur_offset + 0x2])[
            0
        ]  # add offset
        operation = struct.unpack(
            '>B', file_data[cur_offset + 0x2 : cur_offset + 0x3],
        )[0]
        target_section = struct.unpack(
            '>B', file_data[cur_offset + 0x3 : cur_offset + 0x4],
        )[0]
        addend = struct.unpack('>L', file_data[cur_offset + 0x4 : cur_offset + 0x8])[
            0
        ]
        cur_offset += 8

        print(
            'Processing import entry: '
            + format(cur_rel_offset, 'x')
            + ' / '
            + format(operation, 'x')
            + ' / '
            + format(target_section, 'x')
            + ' / '
            + format(addend, 'x'),
        )

        effective_offset = sections[cur_rel_section][0] + cur_rel_offset
        if rel_id == import_entry[0]:
            target_address = sections[target_section][0] + addend + base_address
        else:
            target_address = addend

        print(format(effective_offset, 'x') + ' / ' + format(target_address, 'x'))

        # if operation == 0 or operation == 201: # R_PPC_NONE || R_DOLPHIN_NOP
        # dummy = 0
        if operation == 202:  # R_DOLPHIN_SECTION
            cur_rel_section = target_section
            cur_rel_offset = 0
        elif operation == 1:  # R_PPC_ADDR32
            struct.pack_into('>L', file_data, effective_offset, target_address)
        elif operation == 4:  # R_PPC_ADDR16_LO
            struct.pack_into(
                '>H', file_data, effective_offset, target_address & 0xFFFF,
            )
        elif operation == 6:  # R_PPC_ADDR16_HA
            if (target_address & 0x8000) == 0x8000:
                target_address += 0x00010000

            struct.pack_into(
                '>H', file_data, effective_offset, (target_address >> 16) & 0xFFFF,
            )
        elif operation == 10:  # R_PPC_REL24
            value = addend
            value -= effective_offset + base_address
            orig = struct.unpack(
                '>L', file_data[effective_offset : effective_offset + 0x4],
            )[0]
            orig &= 0xFC000003
            orig |= value & 0x03FFFFFC
            struct.pack_into('>L', file_data, effective_offset, orig)
        elif operation == 203:  # R_DOLPHIN_END
            break
        else:
            print('Unknown relocation operation ' + format(operation, 'x'))

output_data = bytearray(file_data)

filename_out = f'{filename_in}.linked'
with open(filename_out, 'wb') as output:
    output.write(output_data)
