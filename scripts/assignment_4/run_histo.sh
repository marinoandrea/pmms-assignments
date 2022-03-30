#! /usr/bin/bash

declare -r imgs=(
    "1000 100"
    "2000 100"
    "1000 1000"
    "2000 2000"
) 

declare -r img_types=(
    "plasma" 
    "mono" 
    "test"
)

echo -e "input_img\tn_cols\tn_rows\ttime_seq\ttime_krn\ttime_mem"

for img_type in "${img_types[@]}"
do
    for i in {1..5}
    do
        for img in "${imgs[@]}"
        do  
            img_desc_seq=($img)
            m=${img_desc_seq[0]}
            n=${img_desc_seq[1]}

            args="-m $m -n $n -i ../images/${img_type}_${m}x${n}.pgm"
            
            output_lines=($(./histogram/myhistogram $args | tail -4 | head -3 | cut -d= -f2))
            
            output_seq=${output_lines[0]}
            output_krn=${output_lines[2]}
            output_mem=${output_lines[4]}

            result=("${img_type}_${m}x${n}.pgm" "$m" "$n" $output_seq $output_krn $output_mem)

            for field in "${result[@]}"
            do
                echo -ne "$field\t"
            done

            echo ""
        done
    done
done