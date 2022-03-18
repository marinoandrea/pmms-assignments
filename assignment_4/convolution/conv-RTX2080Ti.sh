#!/bin/sh
#SBATCH --time=00:15:00
#SBATCH -N 1
#SBATCH -C RTX2080Ti
#SBATCH --gres=gpu:1

# Not sure what this does, I copied it from https://www.cs.vu.nl/das5/gpu.shtml
. /etc/bashrc
. /etc/profile.d/modules.sh

# Make sure GCC and CUDA modules are loaded regardless of .bashrc contents
# The TAs use CUDA 10, so this makes our results reproducible
module load gcc/6.3.0
module load cuda10.0/toolkit/10.0.130
module load cuda10.0/profiler/10.0.130

# Compile
make clean
make

# Run 10 times
for i in {1..10}
do
   printf "\nRTX 2080 Ti, run %d.\n\n" $i
   ./myconvolution
done