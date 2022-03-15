#!/bin/bash

cd ../assignment_3/histo

# Clean and recompile all mutual exclusion variants
cd with_mutual_ex

cd histo_semaphore
make clean
make
cd ..

# Go back to histogram root directory
cd ..

# Clear output file
> ../../data/raw/histo.txt

echo -e "pat1_5000x5000\n\n" >> ../../data/raw/histo.txt

echo "histo_semaphore" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt

echo -e "\n\npat1_100x150\n\n" >> ../../data/raw/histo.txt

echo "histo_semaphore" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo -e "\n\nareas_100x150\n\n" >> ../../data/raw/histo.txt

echo "histo_semaphore" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo -e "\n\ngradient_100x150.pgm\n\n" >> ../../data/raw/histo.txt

echo "histo_semaphore" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_semaphore/histo_semaphore -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

cd ../../scripts
