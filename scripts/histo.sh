#!/bin/bash

cd ../assignment_3/histo

# Clean and recompile the version without mutual exclusion
cd histo_avoiding_mutual_ex
make clean
make
cd ..

# Clean and recompile all mutual exclusion variants
cd with_mutual_ex

cd histo_mutex
make clean
make
cd ..

cd histo_mutex_many
make clean
make
cd ..

cd histo_semaphores
make clean
make
cd ..

# Go back to histogram root directory
cd ..

# Run performance analysis
hyperfine -m 5 --show-output \
	-n avoiding_mutual_ex "./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16" \
	-n histo_mutex "./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16" \
	-n histo_mutex_many "./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16" \
	-n histo_semaphores "./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16"

cd ../../scripts