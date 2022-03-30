import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns


def main():
    sns.set_theme(style="whitegrid")

    df_old = pd.read_csv("../../data/raw/assignment_3/heat.tsv",
                         sep="\t", index_col=False)
    df_new = pd.read_csv("../../data/raw/assignment_4/heat.tsv",
                         sep="\t", index_col=False)

    df_old['block_size'] = 128
    df_old = df_old[((df_old['n_threads'] == 16) |
                    (df_old['strategy'] == 'seq')) &
                    (df_old['strategy'] != 'pth')]

    df = pd.concat([df_old, df_new], ignore_index=True)
    df['matrix_size'] = df.n_rows.map(int) * df.n_cols.map(int)

    plot = sns.lineplot(x="matrix_size", y="time", hue="strategy",
                        markers=True, dashes=False,
                        data=df, marker='s')

    plot.set_xscale('symlog')
    plot.set_ylabel("Time (seconds)")
    plot.set_xlabel("Matrix Size (area)")

    plt.savefig('heat_matrix_size_vs_time_p16_vs_seq.pdf', dpi=700)
    plt.close()

    df['flops'] /= 1000000000
    df = df[(df['block_size'] == 32) |
            (df['block_size'] == 64) |
            (df['block_size'] == 128)]
    df['matrix_size_txt'] = df.n_rows.map(str) + 'x' + df.n_cols.map(str)

    plot = sns.barplot(x="matrix_size_txt", y="time",
                       hue="block_size", data=df)

    plot.set_yscale('symlog')

    plot.set_xticklabels(df['matrix_size_txt'].unique(),
                         rotation=45, fontsize=6)

    plt.tight_layout()

    plot.set_ylabel("GFLOPs")
    plot.set_xlabel("Matrix Size (row x col)")

    plt.savefig('heat_matrix_size_vs_flops_vs_blocksize.pdf', dpi=700)
    plt.close()

    df = df[df['block_size'] == 128]
    for ms in df['matrix_size_txt'].unique():
        _df = df.copy()
        _df = _df[_df['matrix_size_txt'] == ms]

        seq_mean = df[df['strategy'] == 'seq']['time'].mean()

        def calculate_speedup(time):
            return seq_mean / time

        _df['speedup'] = _df.time.map(calculate_speedup)

        print(f'size: {ms}', 'speedup:', _df['speedup'].max())


if __name__ == '__main__':
    main()
