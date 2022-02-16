#! /usr/bin/python3
import multiprocessing as mp
import os
import re
import subprocess

DEBUG = True  # os.getenv('DEBUG') == 1

INPUT_FOLDER = "../images"

CMD_PREFIX = ["prun", "-np", "1", "-v"] if not DEBUG else []

MAX_ITERS = [10, 100, 1000, 5000, 10_000]

PERIODS = [10, 50, 100]

THRESHOLDS = [1.000000e-05, 1.000000e-04,
              1.000000e-03, 1.000000e-02,
              1.000000e-01, 1.000000]

cc_flags = {
    'gcc': [
        '-O2 -mavx2 -mfma',
        '-O2 -mavx2 -mfma -ffast-math',
        '-O3 -mavx2 -mfma',
        '-O3 -mavx2 -mfma -ffast-math',
    ],
    # 'icc': [

    # ],
    # 'clang': [

    # ],
}


def main():

    results = []

    input_files = list(filter(lambda x: '.pgm' in x, os.listdir(INPUT_FOLDER)))

    for compiler, flags_list in cc_flags.items():
        for flags in flags_list:
            os.rename('heat_seq/Makefile', 'heat_seq/Makefile.backup')
            os.rename('heat_simd/Makefile', 'heat_simd/Makefile.backup')

            create_seq_makefile(compiler, flags)
            create_simd_makefile(compiler, flags)

            process_pool = mp.Pool()
            tasks = []

            try:
                run_build()

                for input_file in input_files:
                    n_cols, n_rows = parse_matrix_size(input_file)

                    for max_iter in MAX_ITERS:
                        for threshold in THRESHOLDS:
                            for period in PERIODS:
                                params = {
                                    "n_cols": n_cols,
                                    "n_rows": n_rows,
                                    "max_iter": max_iter,
                                    "period": period,
                                    "threshold": threshold,
                                    "compiler": compiler,
                                    "flags": flags,
                                    "input_file": input_file,
                                }

                                tasks.extend([
                                    {**params, 'strategy': 'seq'},
                                    {**params, 'strategy': 'simd'},
                                ])

                results.extend(process_pool.map(run_experiment, tasks))
                process_pool.close()
                process_pool.join()

            finally:
                os.rename('heat_seq/Makefile.backup', 'heat_seq/Makefile')
                os.rename('heat_simd/Makefile.backup', 'heat_simd/Makefile')
                tasks.clear()

    with open("PMMS_experiments.tsv", 'w') as f:
        f.write("idx\tstrategy\ttime\ttflops\ttmin\ttmax\ttdif\ttavg\tcompiler\tflags\tn_cols\tn_rows\tperiod\tmax_iter\tthreshold\n")
        for idx, result in enumerate(results):
            out = '\t'.join([
                str(idx),
                result['input_file'].split('/')[-1],
                str(result['strategy']),
                str(result['time']),
                str(result['tflops']),
                str(result['tmin']),
                str(result['tmax']),
                str(result['tdif']),
                str(result['tavg']),
                str(result['compiler']),
                str(result['flags']),
                str(result['n_cols']),
                str(result['n_rows']),
                str(result['period']),
                str(result['max_iter']),
                str(result['threshold']),
            ])
            f.write(f"{out}\n")


def run_experiment(exp_input):
    output = subprocess.check_output([
        *CMD_PREFIX,
        f"./heat_{exp_input.strategy}/heat_{exp_input.strategy}",
        "-m", str(exp_input.n_cols),
        "-n", str(exp_input.n_rows),
        "-i", str(exp_input.max_iter),
        "-e", str(exp_input.threshold),
        "-c", f"{INPUT_FOLDER}/{exp_input.input_file}",
        "-t", f"{INPUT_FOLDER}/{exp_input.input_file}",
        "-k", str(exp_input.period),
        "-L", "0",
        "-H", "100"
    ], text=True)

    last_line = output.splitlines()[-1]
    _, tmin, tmax, tdif, tavg, time, tflops = list(
        map(lambda x: float(x),
            filter(lambda x: x != '', last_line.split(' '))))

    return {
        **exp_input,
        time: time,
        tflops: tflops,
        tmax: tmax,
        tmin: tmin,
        tdif: tdif,
        tavg: tavg
    }


def parse_matrix_size(file_path):
    match = re.search(r'^.*_(\d+)x(\d+).pgm$', file_path)
    if match is None:
        raise ValueError("This file is not a PGM matrix.")
    return int(match.group(1)), int(match.group(2))


def run_build():
    subprocess.check_call(['make', 'clean'], cwd="./heat_seq")
    subprocess.check_call(['make', 'clean'], cwd="./heat_simd")
    subprocess.check_call(['make'], cwd="./heat_seq")
    subprocess.check_call(['make'], cwd="./heat_simd")


def create_seq_makefile(compiler, flags):
    makefile_string = f"""
CURR_DIR=$(notdir $(basename $(shell pwd)))
PRJ=$(CURR_DIR)
SRC=$(filter-out $(wildcard ref*.c), $(wildcard *.c))
OBJ=$(patsubst %.c,%.o,$(SRC))

CC={compiler}
INCLUDES=-I../../include
ifndef DEBUG
CFLAGS={flags} -std=gnu99
LIB=
else
CFLAGS=-O0 -g3 -std=gnu99
LIB=
endif

all: $(PRJ)

$(PRJ): $(OBJ)
\t$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) -o $@ -lm

%.o: %.c
\t$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ $(LIB)

clean:
\t-rm -f $(OBJ) $(PRJ)
    """
    with open('heat_seq/Makefile', 'w') as f:
        f.write(makefile_string)


def create_simd_makefile(compiler, flags):
    makefile_string = f"""
CURR_DIR=$(notdir $(basename $(shell pwd)))
PRJ=$(CURR_DIR)
SRC=$(filter-out $(wildcard ref*.c), $(wildcard *.c))
OBJ=$(patsubst %.c,%.o,$(SRC))

CC={compiler}
INCLUDES=-I../../include
ifndef DEBUG
CFLAGS={flags} -std=gnu99 -mavx2 -mfma
LIB=
else
CFLAGS=-O0 -g3 -std=gnu99 -mavx2 -mfma
LIB=
endif

all: $(PRJ)

$(PRJ): $(OBJ)
\t$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) -o $@ -lm

%.o: %.c
\t$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ $(LIB)

clean:
\t-rm -f $(OBJ) $(PRJ)
    """
    with open('heat_simd/Makefile', 'w') as f:
        f.write(makefile_string)


if __name__ == '__main__':
    main()
