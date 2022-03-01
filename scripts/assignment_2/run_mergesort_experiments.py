#! /usr/bin/python3
import subprocess
import sys
from argparse import ArgumentParser

from numpy import int64

# NOTE: this script must be run with the scripts/assignment_2 folder as CWD
DIR_VECSORT = "../../assignment_2/mergesort"

VERSIONS = [
  'sequential',
  'parallel',
  'parallel_single_task'
]

def main():
    args = parse_arguments(sys.argv[1:])

    run_build(VERSIONS)

    results = []

    for version in VERSIONS:
        params = {
            "length": args.length
        }
        results.append(run_experiment(version, params))

    with open(f"../../data/raw/results_merge_sort.tsv", 'w') as f:
        f.write(
            "idx\tversion\tlength\tadr\ttime\n"
        )
        for idx, result in enumerate(results):
            out = '\t'.join([
                str(idx),
                str(result['version']),
                str(result['length']),
                "random",
                str(result['time'])
            ])
            f.write(f"{out}\n")


def run_experiment(version, exp_input):
    output = subprocess.check_output([
        f"{DIR_VECSORT}/{version}/{version}",
        "-r",
        "-l", str(exp_input['length']),
        "-s", str(42),
        "-p", str(32)
    ]).decode('utf-8')

    return {
        'version': version,
        'length': exp_input['length'],
        'time': output.split(' ')[-2]
    }

def run_build(versions):
    for dir in versions:
      dir = f"{DIR_VECSORT}/{dir}"
      subprocess.check_call(['make', 'clean'], cwd=dir)
      subprocess.check_call(['make'], cwd=dir)

def parse_arguments(args):
    parser = ArgumentParser(description="Run merge sort experiments.")
    parser.add_argument("-l", "--length", type=int, help="Length of the vector.", default=100000000)
    parser.add_argument("-a", "--asc", help="Generate vector values in ascending order")
    parser.add_argument("-d", "--desc", help="Generate vector values in descending order.")
    parser.add_argument("-r", "--rand", help="Generate vector values randomly.")
    return parser.parse_args(args)

if __name__ == '__main__':
    main()
