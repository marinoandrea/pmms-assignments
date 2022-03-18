#!/bin/sh

#SBATCH --time=00:15:00
#SBATCH -N 1

# Other CPU type names that can be used: TitanX-Pascal and RTX2080Ti
#SBATCH -C TitanX
#SBATCH --gres=gpu:1

# Not sure what this does, I copied it from https://www.cs.vu.nl/das5/gpu.shtml
. /etc/bashrc
. /etc/profile.d/modules.sh

# Make sure CUDA modules are loaded regardless of .bashrc contents
module load cuda80/toolkit
module load cuda80/profiler

# Compile and run
nvcc -o convolution convolution.cu
./convolution