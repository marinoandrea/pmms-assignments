#! /usr/bin/python3
"""
This scripts generates random PGM matrix files.
It follow the specification for the P2 version described at:

http://netpbm.sourceforge.net/doc/pgm.html
"""

import random
import sys
from argparse import ArgumentParser

MAX_INT = 65535
MAX_LINE_LENGTH = 70


def main():
    args = parse_arguments(sys.argv[1:])
    width, height = args.cols, args.rows

    out = f"P2\n{width} {height}\n{MAX_INT}\n"

    line_length = 0

    i = 0
    while i < height * width:
        value = str(random.randint(0, MAX_INT))

        if line_length + len(value) + 1 < MAX_LINE_LENGTH:
            out += f'{value} '
            line_length += len(value) + 1
            i += 1
        else:
            out += '\n'
            line_length = 0

    with open(args.output, 'w') as f:
        f.write(out + "\n")


def parse_arguments(args):
    parser = ArgumentParser(description="PGM generation script.")
    parser.add_argument("-n", "--rows", type=int, help="Number of rows.")
    parser.add_argument("-m", "--cols", type=int, help="Number of columns.")
    parser.add_argument("-o", "--output", type=str, help="Output file path.")
    return parser.parse_args(args)


if __name__ == '__main__':
    main()
