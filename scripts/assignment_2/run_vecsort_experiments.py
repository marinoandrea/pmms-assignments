#! /usr/bin/python3
import subprocess
import sys
from argparse import ArgumentParser

# NOTE: this script must be run with the scripts/assignment_2 folder as CWD
DIR_VECSORT = "../../assignment_2/vecsort"

VERSIONS_ADR = [
    'parallel'
]

VERSIONS_INPUT_SIZES = [
    'parallel'
]

VERSIONS_SCHEDULING = [
  'sequential',
  'parallel_static',
  'parallel_dynamic_small_chunk_size',
  'parallel_dynamic_medium_chunk_size',
  'parallel_dynamic_large_chunk_size',
  'parallel_guided_small_chunk_size',
  'parallel_guided_medium_chunk_size',
  'parallel_guided_large_chunk_size'
]

VERSIONS_COMBINED_PARALLELISM = [
  'parallel_combined',
  'sequential_with_parallel_mergesort'
]

VERSIONS_EXTRA = [
  'parallel_presorted',
  'parallel_autovectorized'
]

def main():
    args = parse_arguments(sys.argv[1:])

    if args.versions == 'scheduling':
        versions = VERSIONS_SCHEDULING
    elif args.versions == 'combined_parallelism':
        versions = VERSIONS_COMBINED_PARALLELISM
    elif args.versions == 'extra':
        versions = VERSIONS_EXTRA
    elif args.versions == 'best':
        versions = VERSIONS_BEST

    run_build(versions)

    results = []

    for version in versions:
        for length in args.outer:
            params = {
                "outer": length,
                "inner_min": args.min,
                "inner_max": args.max
            }
            results.append(run_experiment(version, params))

    with open(f"results_{args.versions}.tsv", 'w') as f:
        f.write(
            "idx\tversion\touter\tinner_min\tinner_max\tadr\ttime\n"
        )
        for idx, result in enumerate(results):
            out = '\t'.join([
                str(idx),
                str(result['version']),
                str(result['outer']),
                str(result['inner_min']),
                str(result['inner_max']),
                "random",
                str(result['time'])
            ])
            f.write(f"{out}\n")


def run_experiment(version, exp_input):
    output = subprocess.check_output([
        f"{DIR_VECSORT}/{version}/{version}",
        "-r",
        "-l", str(exp_input['outer']),
        "-n", str(exp_input['inner_min']),
        "-x", str(exp_input['inner_max']),
        "-s", str(42),
        "-p", str(32)
    ]).decode('utf-8')

    return {
        'version': version,
        'outer': exp_input['outer'],
        'inner_min': exp_input['inner_min'],
        'inner_max': exp_input['inner_max'],
        'time': output.split(' ')[-2]
    }

def run_build(versions):
    for dir in versions:
      dir = f"{DIR_VECSORT}/{dir}"
      subprocess.check_call(['make', 'clean'], cwd=dir)
      subprocess.check_call(['make'], cwd=dir)

def parse_arguments(args):
    parser = ArgumentParser(description="Run vecsort experiments.")
    parser.add_argument("-l", "--outer", type=int, nargs='*', help="Lengths of the outer vector.", default=[1000])
    parser.add_argument("-n", "--min", type=int, help="Minimum length of the inner vector.", default = 10)
    parser.add_argument("-x", "--max", type=int, help="Maximum length of the inner vector.", default = 100000)
    parser.add_argument("-a", "--asc", help="Generate inner vector values in ascending order")
    parser.add_argument("-d", "--desc", help="Generate inner vector values in descending order.")
    parser.add_argument("-r", "--rand", help="Generate inner vector values randomly.")
    parser.add_argument("-v", "--versions", type=str, help="List of code version directories to use.")
    return parser.parse_args(args)

if __name__ == '__main__':
    main()
