#! /usr/bin/python3.6

"""Exports symbol information from the .elf built by the REL framework."""
# Jonathan Aldrich 2024-02-23

import os
import subprocess
import sys
from pathlib import Path

import flags    # jdalib

FLAGS = flags.Flags()

FLAGS.DefineString("in_elf", "")
FLAGS.DefineString("in_rel", "")
FLAGS.DefineString("out_dir", "")

class DebugSymbolsError(Exception):
    def __init__(self, message=""):
        self.message = message
        
class SymbolInfo:
    def __init__(self, offset, size, section, ram_address, file_name, symbol_name):
        self.offset = offset
        self.size = size
        self.section = section
        self.ram_address = ram_address
        self.file_name = file_name
        self.symbol_name = symbol_name
        
    def Format(self):
        return "%08x,%08x,%s,%s" % (
            self.ram_address, self.size, self.file_name, self.symbol_name)

def _GenerateRawSymbolsTxt(in_elf, out_dir):
    with open(str(out_dir / 'rel_symbols.txt'), 'w') as outfile:
        completed_process = subprocess.run(
            f"readelf {str(in_elf)} -s -W", stdout=outfile)
        if completed_process.returncode != 0:
            raise DebugSymbolsError(f'readelf failed to read {str(Path)}.')
            sys.exit(1)
    
def _GenerateDebugSymbolCsv(in_rel, out_dir):
    # Get actual RAM addresses for starts of sections.
    section_offsets = {}
    with open(str(in_rel), 'rb') as rel:
        # 1024 bytes should be more than enough for the header.
        rel_header = rel.read(0x400)
        num_sections = int.from_bytes(rel_header[12:16], "big")
        section_tbl = int.from_bytes(rel_header[16:20], "big")
        for x in range(num_sections):
            start = section_tbl + x*8
            offset = int.from_bytes(rel_header[start:start+4], "big") & ~3
            if offset:
                section_offsets[x] = 0x805ba9a0 + offset
                
    # Read symbol information from readelf output.
    symbols = []
    with open(str(out_dir / 'rel_symbols.txt'), 'r') as readelf_file:
        # format:
        #   idx: offset size type binding visibility section name
        line_info = [
            line.strip().split() for line in readelf_file.readlines()[3:]
        ]
        # Parse symbols individually.
        current_file = ""
        for line in line_info:
            try:
                section = int(line[6])
            except ValueError:
                section = None
            if line[3] == "FILE":
                if len(line) < 8:
                    current_file = "gl"
                else:
                    current_file = line[7]
            elif section in section_offsets and line[3] != "SECTION":
                offset = int(line[1], 16)
                size = int(line[2])
                symbol_name = line[7]
                symbols.append(SymbolInfo(
                    offset=offset, size=size, section=section,
                    ram_address=section_offsets[section]+offset,
                    file_name=current_file, symbol_name=symbol_name
                ))

    # Write symbol information to csv in absolute RAM address order.
    symbols.sort(key=lambda x: x.ram_address)
    with open(str(out_dir / 'rel_symbols.csv'), 'w') as outfile:
        for symbol in symbols:
            outfile.write(f"{symbol.Format()}\n")

def main(argc, argv):
    in_elf = FLAGS.GetFlag("in_elf")
    if not in_elf or not os.path.exists(Path(in_elf)):
        raise DebugSymbolsError("--in_elf must point to a valid elf file.")
    in_elf = Path(in_elf)
    
    in_rel = FLAGS.GetFlag("in_rel")
    if not in_rel or not os.path.exists(Path(in_rel)):
        raise DebugSymbolsError("--in_rel must point to a valid rel file.")
    in_rel = Path(in_rel)
    
    out_dir = FLAGS.GetFlag("out_dir")
    if not out_dir or not os.path.exists(Path(out_dir)):
        raise DebugSymbolsError("--out_dir must point to a valid directory.")
    out_dir = Path(out_dir)
    
    _GenerateRawSymbolsTxt(in_elf, out_dir)
    _GenerateDebugSymbolCsv(in_rel, out_dir)


if __name__ == "__main__":
    (argc, argv) = FLAGS.ParseFlags(sys.argv[1:])
    main(argc, argv)