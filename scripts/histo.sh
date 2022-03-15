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

# Clear output file
> ../../data/raw/histo.txt

echo -e "pat1_5000x5000\n\n" >> ../../data/raw/histo.txt

# Run performance analysis
echo "histo_avoiding_mutual_ex" >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt

echo "histo_mutex" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt

echo "histo_mutex_many" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt

echo "histo_semaphores" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt

echo -e "\n\npat1_100x150\n\n" >> ../../data/raw/histo.txt

# Run performance analysis
echo "histo_avoiding_mutual_ex" >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo "histo_mutex" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo "histo_mutex_many" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo "histo_semaphores" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo -e "\n\nareas_100x150\n\n" >> ../../data/raw/histo.txt

# Run performance analysis
echo "histo_avoiding_mutual_ex" >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo "histo_mutex" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo "histo_mutex_many" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo "histo_semaphores" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo -e "\n\ngradient_100x150.pgm\n\n" >> ../../data/raw/histo.txt

# Run performance analysis
echo "histo_avoiding_mutual_ex" >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo "histo_mutex" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex/histo_mutex -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo "histo_mutex_many" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_mutex_many/histo_mutex_many -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo "histo_semaphores" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphores/histo_semaphores -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

cd ../../scripts