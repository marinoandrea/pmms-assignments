import os
import re

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns

INPUT_FILES = list(filter(lambda x: '.tsv' in x, os.listdir('results_heat')))


def parse_matrix_size(row):
    # return f"{ row['n_cols'] }x{ row['n_rows'] }"
    return int(row['n_cols']) * int(row['n_rows'])


def main():
    sns.set_theme(style="whitegrid")

    dfs = []
    for tsv in INPUT_FILES:
        _df = pd.read_csv(f"results_heat/{tsv}", sep="\t")
        match = re.fullmatch(r'results_.*_p([0-9]+).*', tsv)
        _df['n_threads'] = int(match.groups()[0])
        _df.drop('idx', axis=1)
        dfs.append(_df)
    df = pd.concat(dfs, ignore_index=True)

    df['matrix size'] = df.apply(parse_matrix_size, axis=1)

    df_period_1000 = df[df['period'] == 1000]
    df_period_1000_p16 = df_period_1000[df_period_1000['n_threads'] == 16]
    df_period_1000_seq = df_period_1000[
        (df_period_1000['strategy'] == 'seq') &
        (df_period_1000['n_threads'] == 1)]

    plot_seq = sns.lineplot(x="matrix size", y="time",
                            label="Sequential", ci=100,
                            markers=True, dashes=False, linewidth=2.5,
                            data=df_period_1000_seq, marker='s')

    sns.lineplot(x="matrix size", y="time",
                 label="Parallel (OpenMP)", ci=100,
                 markers=True, dashes=False, linewidth=2.5,
                 data=df_period_1000_p16, marker='s')

    plot_seq.set_ylim(0)
    plot_seq.set_xlim(10000)
    plot_seq.set_xscale('symlog')
    plot_seq.set_ylabel("Time (seconds)")
    plot_seq.set_xlabel("Matrix Size (area)")

    plt.title('Sequential vs Parallel heat dissipation computation time')

    plt.savefig('heat_matrix_size_vs_time_p16_vs_seq.pdf', dpi=700)
    plt.close()

    print(
        f"[Space] Time variance (p16): {np.var(df_period_1000_p16['time'][df_period_1000_p16['matrix size'] == 25000000])}")
    print(f"[Space] Time variance (seq): {np.var(df_period_1000_seq['time'])}")

    df_speedup = df_period_1000.copy()
    df_speedup = df_speedup[df_speedup['matrix size'] == 1000000]

    seq_mean = df_speedup[
        (df_speedup['strategy'] == 'seq') &
        (df_speedup['n_threads'] == 1)
    ]['time'].mean()

    def calculate_speedup(row):
        return seq_mean / row['time']

    df_speedup['speedup'] = df_speedup.apply(calculate_speedup, axis=1)

    plot_speedup = sns.lineplot(x="n_threads", y="speedup",
                                markers=True, dashes=False, linewidth=2.5,
                                data=df_speedup, marker='s')

    plot_speedup.set_ylim(1)
    plot_speedup.set_xlim(1)
    plot_speedup.set_ylabel("Speedup (seq. time/ par. time)")
    plot_speedup.set_xlabel("Number of threads")

    print(df_speedup['speedup'])

    plt.title('Heat diffusion Speedup on the 1000x1000 matrix')

    plt.savefig('heat_speedup.pdf', dpi=700)


if __name__ == '__main__':
    main()
