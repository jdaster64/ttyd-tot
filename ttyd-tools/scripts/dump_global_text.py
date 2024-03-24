#! /usr/bin/python3.6

"""Dumps global msg file used by TTYD in update_text_files-friendly format."""
# Jonathan Aldrich 2024-03-24

import os
import subprocess
import sys
from pathlib import Path

import flags    # jdalib

FLAGS = flags.Flags()

FLAGS.DefineString("in_msg_dir", "")
FLAGS.DefineString("out_dir", "")

class DumpGlobalTextError(Exception):
    def __init__(self, message=""):
        self.message = message

def _DumpStrings(in_dir, out_dir):
    strings = []
    if not os.path.exists(in_dir / 'global.txt'):
        raise DumpGlobalTextError("global.txt not found in --in_msg_dir.")
    
    with open(str(in_dir / 'global.txt'), 'rb') as infile:
        # Keys and values are all delimited by nulls.
        split_kvs = infile.read().split(b'\x00')
        x = 0
        # There should be an empty string key at the end of the file.
        while len(split_kvs[x]):
            strings.append((split_kvs[x], split_kvs[x+1]))
            x += 2
            
    outfile_path = str(out_dir / 'global_msg_dump.txt')
    with open(outfile_path, 'w') as outfile:
        for (key, value) in strings:
            outfile.write(f'{key}:\n')
            # Break string into lines by \n characters, and write the value
            # as a implicitly concatenated strings with escaped newlines.
            lines = value.split(b'\n')
            for x in range(len(lines)-1):
                lines[x] += b'\n'
                outfile.write(f'    {lines[x]}\n')
            outfile.write(f'    {lines[-1]},\n\n')
        print(f'Completed successfully; output in {outfile_path}.')
        

def main(argc, argv):
    in_dir = FLAGS.GetFlag("in_msg_dir")
    if not in_dir or not os.path.exists(Path(in_dir)):
        raise DumpGlobalTextError("--in_msg_dir must point to a valid directory.")
    in_dir = Path(in_dir)
    
    out_dir = FLAGS.GetFlag("out_dir")
    if not out_dir or not os.path.exists(Path(out_dir)):
        raise DumpGlobalTextError("--out_dir must point to a valid directory.")
    out_dir = Path(out_dir)
    
    _DumpStrings(in_dir, out_dir)
    


if __name__ == "__main__":
    (argc, argv) = FLAGS.ParseFlags(sys.argv[1:])
    main(argc, argv)