# Assignment 1

## Links

- [Report on Overleaf](https://www.overleaf.com/project/6213a3b36b39b761e5202c98)
- [Assignment description on Canvas](https://canvas.uva.nl/courses/28739/files/folder/Assignments?preview=6182431)
- [Project description on Canvas](https://canvas.uva.nl/courses/28739/files/folder/Assignments?preview=6182439)

## Todo

1. Sequential C base version of heat dissipation simulation
   1. [x] Fix `fmin` and `fmax`
   2. [x] Add time measurements
   3. [x] Generate images
2. Performance impact of compilation
   1. [ ] Run code with `gcc`
   2. [ ] Run code with `icc`
   3. [ ] Run code with other compilers, maybe `clang`? _(optional)_
   4. [ ] Add compiler-specific performance optimizations
   5. [ ] Collect and clean results from compiler comparison
3. Experimentation with sequential heat dissipation code
   1. [ ] Run code with varying parameter sets
   2. [ ] Run code with wide and tall orientations, but the same shape (N x M versus M x N)
5. Vectorisation (SIMD)
   1. [ ] Hand-vectorize sequential code
   2. [ ] Automatically vectorize using compiler flags, pragmas, code rewriting
   3. [ ] Compare sequential to hand-vectorized to auto-vectorized
6. Report
   1. [ ] Intro
   2. [ ] Experimental results
   3. [ ] Graphs
   4. [ ] ...

## Useful commands

### SSH config

Add the following to your SSH config file (usually `~/.ssh/config`) to work on the DAS-5 from home:

```
Host vu
    Hostname 130.37.164.129
    User <your VUnetID>

Host das5
    Hostname fs0.das5.cs.vu.nl
    User <your DAS-5 username>
    ProxyCommand ssh -q vu nc %h 22
```

You'll still have to enter both of your passwords every time you log in (first your VU password, then your DAS-5 account password).

### Local

A list of all installed compiler intrinsics for GCC, everything that is not present has `-mno-` in front. This may also work on DAS-5 but make sure you run it on a compute node, not on the head node.

```bash
alias intrinsics="gcc -v -xc /dev/null -O3 -march=native -o- -E 2>&1 | grep -o -- '-m[-a-z0-9=]*'"
```

Clean, make, and run with a single command (sequential):

```bash
make clean && make && ./heat_seq -n 150 -m 100 -i 42 -e 0.0001 -c ../../images/pat1_100x150.pgm -t ../../images/pat1_100x150.pgm -r 1 -k 10 -L 0 -H 100
```

Clean, make, and run with a single command (SIMD):

```bash
make clean && make && ./heat_simd -n 150 -m 100 -i 42 -e 0.0001 -c ../../images/pat1_100x150.pgm -t ../../images/pat1_100x150.pgm -r 1 -k 10 -L 0 -H 100
```

### DAS-5

Always load `prun`, CUDA, and ICC when you log in:

```bash
module load prun
module load cuda80/toolkit
module load intel/compiler/64/15.0/2015.5.223
```

Just some shorthand commands:

```bash
alias nodes="sinfo -o '%40N %40f'"
alias reservations="preserve -llist"
```

You can rename them if you want or add your own.
