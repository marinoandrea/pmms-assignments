# Assignment 4 - GPU programming and heterogeneous computing

_Submission due date: **Tuesday, March 29, 2022**._

## Links

- [Report on Overleaf](https://www.overleaf.com/project/62347b9bdb596043b564c8b7)
- [Assignment description on Canvas](https://canvas.uva.nl/courses/28739/files/folder/Assignments?preview=6385699)
- [Project description on Canvas](https://canvas.uva.nl/courses/28739/files/folder/Assignments?preview=6182439)
- [DAS 5 cheat-sheet](./DAS5_cheatsheet.md)

## Todo list

1. [ ] Assignment 4.1: **Heat dissipation**
   - [ ] Choose suitable kernels
   - [ ] Implement CUDA variant
   - [ ] *(optional)* Optimization: shared memory
   - [ ] *(optional)* Optimization: improve caching
   - [ ] Verify correctness
   - [ ] Measure performance of the GPU implementation(s)
   - [ ] Measure performance of all CPU implementations
   - [ ] Data analysis
thus targeting an NVIDIA GPU
2. [ ] Assignment 4.2: **Histogram** in CUDA
   - [ ] Design at least two different GPU implementations
   - [ ] Implement all GPU variants
   - [ ] Verify correctness
   - [ ] Measure performance of all GPU implementations
   - [ ] Measure performance of all CPU implementations
   - [ ] Data analysis
3. [ ] Assignment 4.3: **Smooting filter**
   - [ ] Parallelize smoothing filter using CUDA
   - [ ] Optimize kernel as much as possible
   - [ ] Measure performance for at least 5 different images
   - [ ] Verify correctness
   - [ ] Data analysis
4. [ ] Assignment 4.4: **Smoothing filter** revisited *(bonus assignment)*
   - [ ] Implement smoothing filter using a differnent programming model or by using heterogeneous computing
   - [ ] Verify correctness
   - [ ] Measure performance for at least 5 different images
   - [ ] Data analysis

## Running GPU jobs on DAS-5

Create a job script. It's just a shell script with some extra SLURM `sbatch` syntax at the top. The file extension doesn't matter, I've seen `.gpu`, `.job`, `.sh` or no extension at all. Add the following lines at the top of the file:

```
#!/bin/sh
#SBATCH --time=00:15:00
#SBATCH -N 1
#SBATCH -C RTX2080Ti
#SBATCH --gres=gpu:1
```

You can change the GPU model here if you want. If you don't care about the GPU model as long as there is one in the first place, you can leave it out and just use `-C gpunode` instead of the GPU model name. Leave a line blank and add the contents of the file. I use this as boilerplate code:

```
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
```
