# Assignment 3 - Multi-core programming with `pthreads` and beyond

_Submission due date: **Tuesday, March 15, 2022**._

## Links
- [Report on Overleaf](https://www.overleaf.com/project/6221f5d79863100f8b299f8c)
- [Assignment description on Canvas](https://canvas.uva.nl/courses/28739/files/folder/Assignments?preview=6305473)
- [Project description on Canvas](https://canvas.uva.nl/courses/28739/files/folder/Assignments?preview=6182439)
- [DAS 5 cheat-sheet](./DAS5_cheatsheet.md)

## Todo list

1. [ ] Assignment 3.1: Parallelisation of heat dissipation code with `pthreads`
   - [ ] Implement parallel heat dissipation simulation with `pthreads`
   - [ ] (_optional_) Experiment with different design choices
2. [ ] Assignment 3.2: Experimentation with `pthreads` heat dissipation code
   - [ ] Generate experimental results
   - [ ] Make graphs
3. [ ] Assignment 3.3: Pipeline parallelism using Posix Threads
   - [ ] Implement pipeline sort
   - [ ] Verify correctness
4. [ ] Assignment 3.4: Mutual exclusion concepts
   - [ ] Implement image histogram algorithm with mutex locks
   - [ ] Implement image histogram algorithm with semaphores
   - [ ] Implement image histogram algorithm with atomic operations
   - [ ] Implement image histogram algorithm with software transactional memory
   - [ ] Implement image histogram algorithm with multiple sets of bins
   - [ ] Verify correctness of all five variants
5. [ ] Assignment 3.5: Experimentation with mutual exclusion concepts
   - [ ] Design experimental setup
   - [ ] Identify all parameters (application and input) that can impact performance
   - [ ] Write hypothesis for performance impact of each identified parameter
   - [ ] Write a script to test many hypotheses and program variants easily
   - [ ] Test hypotheses and generate experimental results
   - [ ] Analyze results and make graphs
   - [ ] Rank mutual exclusion concepts by relative performance and discuss advantages/disadvantages

## Assignment details

The CLI of the heat dissipation part of the assignment is the same as for assignment 1 & 2. 

The CLI for the histogram part is as follows:
 - **-s** sets the seed (default 42)
 - **-i** image path
 - **-r** if set then generate a random image (default False)
 - **-p** sets the number of threads used (default 1)
 - **-n** number of rows of the image
 - **-m** number of columns of the image
 - **-g** debug (print historgram)

You may change everything in the boilerplate code. The only thing that **HAS TO** stay the same is the current options in the CLI and the output this produces. So you may change things like the data structures used **BUT** we still want the same output when printing the histogram. You may also extend the functionality (i.e. if you want to generate images of the same color you can write a second "generate image" function). We will only test your code using the images in the "images" folder.  

The CLI for the pipesort is as follows:
 - **-l** sets the length of the vector
 - **-s** sets the seed of the random number generator
 You may extend the CLI, but not alter the behaviour of "l" and "s". Set any additional flags to the default you want us to use. 


1) Assignment 3 - Histogram Testing: TODO

    For correctness:
        "- i ../../../images/pat1_100x150.pgm -n 150 -m 100 -g -p 2"
        
    For performance:
        "- i ../../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16"

2) Assignment 3 - Pipesort Testing:

    For correctness:
         "- l 1000 -s 42"
        
    For performance:
        "- l 50000 -s 42"
        
    To push your code we will use (comment: have to see if 1000000 is reasonable):
        "- l 1000000 -s 42"
