#! /usr/bin/python3
import subprocess
import sys
from argparse import ArgumentParser

# NOTE: this script must be run with the scripts/assignment_2 folder as CWD
DIR_VECSORT = "../../assignment_2/vecsort"

VERSIONS_SCHEDULING = [
  'sequential',
  'parallel_static',
  'parallel_dynamic_small_chunks',
  'parallel_dynamic_medium_chunks',
  'parallel_dynamic_large_chunks',
  'parallel_guided_small_chunks',
  'parallel_guided_medium_chunks',
  'parallel_guided_large_chunks'
]

VERSIONS_COMBINED_PARALLELISM = [
  # best parallel
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

    run_build(versions)

    results = []

    for version in versions:
        for length in args.outer:
            params = {
                "outer": length,
                "inner_min": args.min,
                "inner_max": args.max,
                "adr": args.adr
            }
            results.append(run_experiment(version, params))

    with open(f"results_{versions}.tsv", 'w') as f:
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
                str(result['adr'])
            ])
            f.write(f"{out}\n")


def run_experiment(exp_dir, exp_input):
    return subprocess.check_output([
        f"{DIR_VECSORT}/{exp_dir}",
        "-l", str(exp_input['outer']),
        "-n", str(exp_input['inner_min']),
        "-x", str(exp_input['inner_max']),
        "-adr", str(exp_input['adr'])
    ]).decode('utf-8')

def run_build(versions):
    for dir in versions:
      dir = f"{DIR_VECSORT}/{dir}"
      subprocess.check_call(['make', 'clean'], cwd=dir)
      subprocess.check_call(['make'], cwd=dir)

def parse_arguments(args):
    parser = ArgumentParser(description="Run vecsort experiments.")
    parser.add_argument("-l", "--outer", type=int, nargs='*', help="Lengths of the outer vector.")
    parser.add_argument("-n", "--min", type=int, help="Minimum length of the inner vector.")
    parser.add_argument("-x", "--max", type=int, help="Maximum length of the inner vector.")
    parser.add_argument("-a", "--adr", type=int, help="Generate inner vector values in ascending order, descending order, or randomly.")
    parser.add_argument("-d", "--versions", type=str, help="List of directories to use.")
    return parser.parse_args(args)

if __name__ == '__main__':
    main()
