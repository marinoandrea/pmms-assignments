#!/bin/sh
#SBATCH --time=00:15:00
#SBATCH -N 1
#SBATCH -C TitanX
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

# Four 100x150 images...
for image in {pat1_100x150.pgm, pat2_100x150.pgm, pat3_100x150.pgm, plasma_100x150.pgm}
do
   for i in {1..100}
   do
      printf "\nTitanX Maxwell, file %s, run %d.\n\n" $image, $i
      ./myconvolution -i ../../images/$image -h 100 -w 150
   done
done

# And one 2000 x 2000 image.
for i in {1..100}
do
   printf "\nTitanX Maxwell, file test_2000x2000.pgm, run %d.\n\n" , $i
   ./myconvolution -i ../../images/test_2000x2000.pgm -h 2000 -w 2000
done