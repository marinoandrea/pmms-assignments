#! /usr/bin/bash

args="-i 10000 -e 0.0001 -r 1 -k 70 -L 0 -H 100"

declare -r imgs=(
    "50 100"
    "100 50"
    "100 200"
    "200 100"
    "100 1000"
    "1000 100"
    "100 2000"
    "2000 100"
    "1000 1000"
    "2000 2000"
) 

declare -r block_sizes=(32 64 128)

echo -e "strategy\tblock_size\tn_cols\tn_rows\titerations\tmin\tmax\tdiff\tavg\ttime\tflops"

for block_size in "${block_sizes[@]}"
do
    sed -i "11s/.*/#define BLOCK_SIZE $block_size/" ./heat_cuda/cuda_compute.cu

    cd heat_cuda && make clean 1> /dev/null && make 1> /dev/null && cd ..

    for i in {1..5}
    do
        for img in "${imgs[@]}"
        do  
            img_desc_seq=($img)
            m=${img_desc_seq[0]}
            n=${img_desc_seq[1]}

            additional_args="-m $m -n $n -c ../images/test_${m}x${n}.pgm -t ../images/test_${m}x${n}.pgm"

            output_cuda=("cuda" "$block_size" "$m" "$n" $(./heat_cuda/heat_cuda $args $additional_args | tail -1))

            for field in "${output_cuda[@]}"
            do
                echo -ne "$field\t"
            done

            echo ""
        done
    done
done