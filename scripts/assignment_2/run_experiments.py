#! /usr/bin/python3
import re
import subprocess
import sys
from argparse import ArgumentParser

# NOTE: this script must be run with the scripts/assignment_2 folder as CWD
DIR_PGM = "../../images"
DIR_SEQ = "../../assignment_1/heat_seq"
DIR_OMP = "../../assignment_2/heat_omp"

MAX_ITERS = 10000
PERIODS = [50, 1000]


def main():
    args = parse_arguments(sys.argv[1:])
    N, M = args.rows, args.cols
    n_threads = args.threads

    run_build()

    results = []

    n_cols, n_rows = parse_matrix_size(f"{DIR_PGM}/omp_{M}x{N}.pgm")

    for period in PERIODS:
        params = {
            "n_cols": n_cols,
            "n_rows": n_rows,
            "max_iter": MAX_ITERS,
            "period": period,
            "threshold": 0.0001,
            "input_file": f"omp_{M}x{N}.pgm",
            "n_threads": n_threads
        }
        results.append(run_experiment(
            {**params, "strategy": "seq"}))
        results.append(run_experiment(
            {**params, "strategy": "omp"}))

    with open(f"results_{M}x{N}_p{n_threads}.tsv", 'w') as f:
        f.write(
            "idx\tinput_file\tstrategy\tn_threads\ttime\ttflops\tn_cols"
            + "\tn_rows\tperiod\tmax_iter\tthreshold\n"
        )
        for idx, result in enumerate(results):
            out = '\t'.join([
                str(idx),
                result['input_file'].split('/')[-1],
                str(result['strategy']),
                str(result['n_threads']),
                str(result['time']),
                str(result['tflops']),
                str(result['n_cols']),
                str(result['n_rows']),
                str(result['period']),
                str(result['max_iter']),
                str(result['threshold']),
            ])
            f.write(f"{out}\n")


def run_experiment(exp_input):
    output = subprocess.check_output([
        (f"{DIR_OMP}/heat_omp"
         if exp_input['strategy'] == 'omp'
         else f"{DIR_SEQ}/heat_seq"),
        "-m", str(exp_input['n_cols']),
        "-n", str(exp_input['n_rows']),
        "-i", str(exp_input['max_iter']),
        "-e", str(exp_input['threshold']),
        "-c", f"{DIR_PGM}/{exp_input['input_file']}",
        "-t", f"{DIR_PGM}/{exp_input['input_file']}",
        "-k", str(exp_input['period']),
        "-L", "0",
        "-H", "100",
        "-p", str(exp_input['n_threads'])
    ]).decode('utf-8')

    last_line = output.splitlines()[-1]
    _, tmin, tmax, tdif, tavg, time, tflops = list(
        map(lambda x: float(x),
            filter(lambda x: x != '', last_line.split(' '))))

    return {
        **exp_input,
        'time': time,
        'tflops': tflops,
        'tmax': tmax,
        'tmin': tmin,
        'tdif': tdif,
        'tavg': tavg
    }


def parse_matrix_size(file_path):
    match = re.search(r'^.*_(\d+)x(\d+).pgm$', file_path)
    if match is None:
        raise ValueError("This file is not a PGM matrix.")
    return int(match.group(1)), int(match.group(2))


def run_build():
    subprocess.check_call(['make', 'clean'], cwd=DIR_SEQ)
    subprocess.check_call(['make'], cwd=DIR_SEQ)
    subprocess.check_call(['make', 'clean'], cwd=DIR_OMP)
    subprocess.check_call(['make'], cwd=DIR_OMP)


def parse_arguments(args):
    parser = ArgumentParser(description="PGM generation script.")
    parser.add_argument("-n", "--rows", type=int, help="Number of rows.")
    parser.add_argument("-m", "--cols", type=int, help="Number of columns.")
    parser.add_argument("-p", "--threads", type=int, help="Number of threads.")
    return parser.parse_args(args)


if __name__ == '__main__':
    main()
