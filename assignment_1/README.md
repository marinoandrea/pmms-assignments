# Assignment 1

## Links

- [Assignment description on Canvas](https://canvas.uva.nl/courses/28739/files/folder/Assignments?preview=6182431)
- [Project description on Canvas](https://canvas.uva.nl/courses/28739/files/folder/Assignments?preview=6182439)

## Useful commands

### Local

A list of all installed compiler intrinsics for GCC, everything that is not present has `-mno-` in front. This may also work on DAS-5 but make sure you run it on a compute node, not on the head node.

```bash
alias intrinsics="gcc -v -xc /dev/null -O3 -march=native -o- -E 2>&1 | grep -o -- '-m[-a-z0-9=]*'"
```
### DAS-5

Always load `prun` and CUDA when you log in:

```bash
module load prun
module load cuda80/toolkit
```

Just some shorthand commands:

```bash
alias nodes="sinfo -o '%40N %40f'"
alias reservations="preserve -llist"
```

You can rename them if you want or add your own.

## Todo

1. Sequential C base version of heat dissipation simulation

   1. [ ] Fix `fmin` and `fmax`
   2. [ ] Add time measurements
   3. [ ] Generate images

2. Performance impact of compilation

   1. [ ] Run code with `gcc`
   2. [ ] Run code with `icc`
   3. [ ] Run code with other compilers, maybe `clang`? _(optional)_
   4. [ ] Add compiler-specific performance optimizations

3. Experimentation with sequential heat dissipation code

   1. [ ] Collect and clean results from compiler comparison
   2. [ ] Run code with varying parameter sets
   3. [ ] Run code with wide and tall orientations, but the same shape (N x M versus M x N)

5. Vectorisation (SIMD)

   1. [ ] Hand-vectorize sequential code
   2. [ ] Automatically vectorize using compiler flags, pragmas, code rewriting
   3. [ ] Compare sequential to hand-vectorized to auto-vectorized

6. Report

   1. [ ] Intro
   2. [ ] Experimental results
   3. [ ] Graphs