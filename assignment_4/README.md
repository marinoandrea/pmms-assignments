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