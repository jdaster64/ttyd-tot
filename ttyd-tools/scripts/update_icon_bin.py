#! /usr/bin/python3.6

"""Updates icon.bin to be able to reference new icon images in icon.tpl."""
# Jonathan Aldrich 2024-03-08

import os
import subprocess
import sys
from pathlib import Path

import bindatastore as bd   # jdalib
import flags                # jdalib

FLAGS = flags.Flags()

FLAGS.DefineString("in_icon_bin", "")
FLAGS.DefineString("out_dir", "")

# icon.bin format:
# u16 num_icons
# u16 icon_data_offsets[num_icons]
# icon data format:
# u16 is_animated
# u16 unknown? '0x30' for icon 348
# u16 unknown? '0x30' for icon 348
# u16 num_animation_frames
# array of frame data:
# u16 icon_id
# u16 frame_duration (0 for non-animated)

g_NewIconDefs = [
    # Blue coin
    [(613, 6), (614, 6), (615, 6), (616, 6)],
    # Super Start badge
    [(617, 0)],
    # Perfect Power badge
    [(618, 0)],
    # Perfect Power P badge
    [(619, 0)],
    # Timer colon
    [(620, 0)],
    # Timer decimal point
    [(621, 0)],
    # White square
    [(622, 0)],
    # Pity Star badge
    [(623, 0)],
    # Pity Star P badge
    [(624, 0)],
    # Hottest Dog item
    [(625, 0)],
]
g_IconDefs = []

class UpdateIconBinError(Exception):
    def __init__(self, message=""):
        self.message = message

def _ParseOriginalIconBin(in_file):
    dat = bd.BDStore(big_endian=True)
    dat.RegisterFile(str(in_file))
    
    num_icons = dat.view(0).ru16(0)
    idat = dat.view((num_icons + 1)* 2)
    
    for _ in range(num_icons):
        icon_frames = []
        # Extract existing frame information.
        num_frames = idat.ru16(6)
        for f in range(num_frames):
            icon_frames.append((idat.ru16(4*f + 8), idat.ru16(4*f + 10)))
        g_IconDefs.append(icon_frames)
        # Advance to next icon's data.
        idat = idat.at(4 * num_frames + 8)
        
def _WriteU16(outfile, num):
    outfile.write(bytes([num >> 8, num & 0xff]))
        
def _WriteModifiedIconBin(out_dir):
    # Combine lists of icon defintions.
    for idef in g_NewIconDefs:
        g_IconDefs.append(idef)

    # Precalculate offsets.
    offsets = []
    offset = (len(g_IconDefs) + 1) * 2
    for idef in g_IconDefs:
        offsets.append(offset)
        offset += 4 * len(idef) + 8

    with open(str(out_dir / f'icon.bin'), 'wb') as f:
        _WriteU16(f, len(offsets))
        for x in offsets:
            _WriteU16(f, x)
        for idef in g_IconDefs:
            _WriteU16(f, 1 if len(idef) else 0)
            _WriteU16(f, 0x30 if idef[0][0] == 0x161 else 0)
            _WriteU16(f, 0x30 if idef[0][0] == 0x161 else 0)
            _WriteU16(f, len(idef))
            for frame in idef:
                _WriteU16(f, frame[0])
                _WriteU16(f, frame[1])

def main(argc, argv):
    in_file = FLAGS.GetFlag("in_icon_bin")
    if not in_file or not os.path.exists(Path(in_file)):
        raise UpdateTextFilesError("--in_icon_bin must point to a valid icon.bin.")
    in_file = Path(in_file)
    
    out_dir = FLAGS.GetFlag("out_dir")
    if not out_dir or not os.path.exists(Path(out_dir)):
        raise UpdateTextFilesError("--out_dir must point to a valid directory.")
    out_dir = Path(out_dir)
    
    _ParseOriginalIconBin(in_file)
    _WriteModifiedIconBin(out_dir)


if __name__ == "__main__":
    (argc, argv) = FLAGS.ParseFlags(sys.argv[1:])
    main(argc, argv)