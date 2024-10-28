#! /usr/bin/python3.6

"""Exports symbol information from the .elf built by the REL framework."""
# Jonathan Aldrich 2024-02-23

import os
import subprocess
import sys
from pathlib import Path

import flags  # jdalib

FLAGS = flags.Flags()

FLAGS.DefineString('in_elf', '')
FLAGS.DefineString('in_rel', '')
FLAGS.DefineString('out_dir', '')


class DebugSymbolsError(Exception):
    def __init__(self, message: str = '') -> None:
        self.message = message


class SymbolInfo:
    def __init__(
        self,
        offset: int,
        size: int,
        section: int,
        ram_address: int,
        file_name: str,
        symbol_name: str,
    ) -> None:
        self.offset = offset
        self.size = size
        self.section = section
        self.ram_address = ram_address
        self.file_name = file_name
        self.symbol_name = symbol_name

    def format(self) -> str:
        return ','.join(
            [
                f'{self.ram_address:08x}',
                f'{self.size:08x}',
                f'{self.file_name}',
                f'{self.symbol_name}',
            ],
        )


def _generate_raw_symbols_txt(in_elf: ..., out_dir: Path) -> None:
    with open(str(out_dir / 'rel_symbols.txt'), 'w') as outfile:
        completed_process = subprocess.run(
            f'readelf {in_elf!s} -s -W', stdout=outfile, check=False,
        )
        if completed_process.returncode != 0:
            msg = f'readelf failed to read {Path!s}.'
            raise DebugSymbolsError(msg)


def _generate_debug_symbol_csv(in_rel: ..., out_dir: Path) -> None:
    # Get actual RAM addresses for starts of sections.
    section_offsets = {}
    with open(str(in_rel), 'rb') as rel:
        # 1024 bytes should be more than enough for the header.
        rel_header = rel.read(0x400)
        num_sections = int.from_bytes(rel_header[12:16], 'big')
        section_tbl = int.from_bytes(rel_header[16:20], 'big')
        for x in range(num_sections):
            start = section_tbl + x * 8
            if offset := int.from_bytes(rel_header[start : start + 4], 'big') & ~3:
                section_offsets[x] = 0x805BA9A0 + offset

    # Read symbol information from readelf output.
    symbols = []
    with open(str(out_dir / 'rel_symbols.txt')) as readelf_file:
        # format:
        #   idx: offset size type binding visibility section name
        line_info = [line.strip().split() for line in readelf_file.readlines()[3:]]
        # Parse symbols individually.
        current_file = ''
        for line in line_info:
            try:
                section = int(line[6])
            except ValueError:
                section = None
            if line[3] == 'FILE':
                current_file = 'gl' if len(line) < 8 else line[7]
            elif section in section_offsets and line[3] != 'SECTION':
                offset = int(line[1], 16)
                size = int(line[2])
                symbol_name = line[7]
                symbols.append(
                    SymbolInfo(
                        offset=offset,
                        size=size,
                        section=section,
                        ram_address=section_offsets[section] + offset,
                        file_name=current_file,
                        symbol_name=symbol_name,
                    ),
                )

    # Write symbol information to csv in absolute RAM address order.
    symbols.sort(key=lambda x: x.ram_address)
    with open(str(out_dir / 'rel_symbols.csv'), 'w') as outfile:
        for symbol in symbols:
            outfile.write(f'{symbol.Format()}\n')


def _ensure_flag_exists(flag: ..., msg: str) -> Path:
    result = FLAGS.GetFlag(flag)
    if not result or not Path(result).exists():
        raise DebugSymbolsError(msg)
    return Path(result)



def main(argc: ..., argv: ...) -> None:
    """Main entry point."""
    in_elf = _ensure_flag_exists(
        'in_elf', '--in_elf must point to a valid elf file.',
    )
    in_rel = _ensure_flag_exists(
        'in_rel', '--in_rel must point to a valid rel file.',
    )
    out_dir = _ensure_flag_exists(
        'out_dir', '--out_dir must point to a valid directory.',
    )
    _generate_raw_symbols_txt(in_elf, out_dir)
    _generate_debug_symbol_csv(in_rel, out_dir)


if __name__ == '__main__':
    (argc, argv) = FLAGS.ParseFlags(sys.argv[1:])
    main(argc, argv)
