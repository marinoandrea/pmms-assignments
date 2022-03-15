#!/bin/bash

cd ../assignment_3/histo

# Clean and recompile all mutual exclusion variants
cd with_mutual_ex

cd histo_atomic
make clean
make
cd ..

cd histo_sw_transactional
make clean
make
cd ..

# Go back to histogram root directory
cd ..

# Clear output file
> ../../data/raw/histo.txt

echo -e "pat1_5000x5000\n\n" >> ../../data/raw/histo.txt

echo "histo_atomic" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt

echo -e "\n\npat1_100x150\n\n" >> ../../data/raw/histo.txt

echo "histo_atomic" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo -e "\n\nareas_100x150\n\n" >> ../../data/raw/histo.txt

echo "histo_atomic" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo -e "\n\ngradient_100x150.pgm\n\n" >> ../../data/raw/histo.txt

echo "histo_atomic" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_atomic/histo_atomic -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo -e "pat1_5000x5000\n\n" >> ../../data/raw/histo.txt

echo "histo_sw_transactional" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/pat1_5000x5000.pgm -n 5000 -m 5000 -p 16 >> ../../data/raw/histo.txt

echo -e "\n\npat1_100x150\n\n" >> ../../data/raw/histo.txt

echo "histo_sw_transactional" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/pat1_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo -e "\n\nareas_100x150\n\n" >> ../../data/raw/histo.txt

echo "histo_sw_transactional" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/areas_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

echo -e "\n\ngradient_100x150.pgm\n\n" >> ../../data/raw/histo.txt

echo "histo_sw_transactional" >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt
./with_mutual_ex/histo_sw_transactional/histo_sw_transactional -i ../../images/gradient_100x150.pgm -m 100 -n 150 -p 16 >> ../../data/raw/histo.txt

cd ../../scripts
