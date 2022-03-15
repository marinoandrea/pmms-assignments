import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns


def main():
    sns.set_theme(style="whitegrid")

    df = pd.read_csv("../../data/raw/assignment_3/pipesort.tsv", sep="\t")

    plot = sns.lineplot(x="input_size", y="time_s", hue="buffer_size",
                        dashes=False, linewidth=1, markers=False,
                        data=df, palette="Accent")

    plot.set_ylim(0)
    plot.set_xlim(16, 30000)
    plot.set_ylabel("Time (seconds)")
    plot.set_xlabel("Sequence length / # Threads")

    plt.title("Pipesort Execution Time VS Sequence Length")
    plt.savefig('pipesort.pdf', dpi=700)
    plt.close()


if __name__ == '__main__':
    main()
