import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns


def main():
    sns.set_theme(style="whitegrid")

    df = pd.read_csv("../../../data/raw/assignment_3/heat.tsv",
                     sep="\t", index_col=False)

    df['matrix_size'] = df.n_rows.map(int) * df.n_cols.map(int)

    df_p16 = df[(df['n_threads'] == 16) | (df['strategy'] == 'seq')]

    plot = sns.lineplot(x="matrix_size", y="time", hue="strategy",
                        markers=True, dashes=False,
                        data=df_p16, marker='s')

    plot.set_xscale('symlog')
    plot.set_ylabel("Time (seconds)")
    plot.set_xlabel("Matrix Size (area)")

    plt.title('Sequential vs Parallel heat dissipation computation time')

    plt.savefig('heat_matrix_size_vs_time_p16_vs_seq.pdf', dpi=700)
    plt.close()

    df_speedup = df.copy()
    df_speedup = df_speedup[df_speedup['matrix_size'] == 1000000]

    seq_mean = df_speedup[df_speedup['strategy'] == 'seq']['time'].mean()

    def calculate_speedup(row):
        return seq_mean / row['time']

    df_speedup['speedup'] = df_speedup.apply(calculate_speedup, axis=1)

    df_speedup_noseq = df_speedup[df_speedup['strategy'] != 'seq']

    plot_speedup = sns.lineplot(x="n_threads", y="speedup", hue="strategy",
                                markers=True, dashes=False,
                                data=df_speedup_noseq, marker='s')

    plot_speedup.set_ylim(1)
    plot_speedup.set_xlim(1)
    plot_speedup.set_ylabel("Speedup (seq. time/ par. time)")
    plot_speedup.set_xlabel("Number of threads")

    plt.title('Heat diffusion Speedup on the 1000x1000 matrix')

    plt.savefig('heat_speedup.pdf', dpi=700)


if __name__ == '__main__':
    main()
