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

declare -r n_threads=(2 4 8 16 32) 

echo -e "strategy\tn_threads\tn_cols\tn_rows\titerations\tmin\tmax\tdiff\tavg\ttime\tflops"


for nt in "${n_threads[@]}"
do
    for img in "${imgs[@]}"
    do  
        img_desc_seq=($img)
        m=${img_desc_seq[0]}
        n=${img_desc_seq[1]}

        additional_args="-m $m -n $n -c ../images/test_${m}x${n}.pgm -t ../images/test_${m}x${n}.pgm -p $nt"

        if [[ $nt == 2 ]]
        then 
            output_seq=("seq" "$nt" "$m" "$n" $(./heat_seq/heat_seq $args $additional_args | tail -1))
            
            for field in "${output_seq[@]}"
            do
                echo -ne "$field\t"
            done

            echo ""
        fi
        
        output_pth=("pth" "$nt" "$m" "$n" $(./heat_pth/heat_pth $args $additional_args | tail -1))
        output_omp=("omp" "$nt" "$m" "$n" $(./heat_omp/heat_omp $args $additional_args | tail -1))


        for field in "${output_pth[@]}"
        do
            echo -ne "$field\t"
        done

        echo ""

        for field in "${output_omp[@]}"
        do
            echo -ne "$field\t"
        done
        
        echo ""

    done
done