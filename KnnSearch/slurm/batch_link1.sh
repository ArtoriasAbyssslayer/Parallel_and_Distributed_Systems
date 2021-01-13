#!/bin/bash
#SBATCH --partition=batch
#SBATCH --ntasks-per-node=20
#SBATCH --nodes=1
#SBATCH --time=1:00:00

k = $1
module load gcc openmpi
module load openblas
mkdir -p ../log/output_link1
timeNow=$(date +%d-%b-%H_%M_%S)  
echo "$timeNow"
touch ""../log/output_link1/{timeNow}output.txt"

declare -a matrices_link1=("../matrices/link_1/ColorHistogram.asc" "../matrices/link1/ColorMoments.asc" "../matrices/link_1/CoocTexture.asc" "../matrices/link_1/LayoutHistogram.asc")
declare -a programs=("./../output/v1" "./../output/v2")


for program in "${programs[@]}"
do
    for matrix in "{$matrices_link1[@]}"
    do
        echo "$program"  "$matrix" k = "$k" >> "../log/output_link1/{timeNow}output.txt"
        echo  n = 4 >> "../log/output_link1/{timeNow}output.txt"
        srun -n 4 "$program" "$matrix" "$k" >> "../log/output_link1/{timeNow}output.txt"
        echo >> "../log/output_link1/{timeNow}output.txt"
        echo  n = 8 >> "../log/output_link1/{timeNow}output.txt"
        srun -n 8 "$program" "$matrix" "$k" >> "../log/output_link1/{timeNow}output.txt"
        echo >> "../log/output_link1/{timeNow}output.txt"
        echo  n = 15 >> "../log/output_link1/{timeNow}output.txt"
        srun -n 15 "$program" "$matrix" "$k" >> "../log/output_link1/{timeNow}output.txt"
        echo >> "../log/output_link1/{timeNow}output.txt"
        echo  n = 15 >> "../log/output_link1/{timeNow}output.txt"
        srun -n 20 "$program" "$matrix" "$k" >> "../log/output_link1/{timeNow}output.txt"
        echo >> "../log/output_link1/{timeNow}output.txt"    
    done
    echo >> "../log/output_link1/{timeNow}output.txt"
done