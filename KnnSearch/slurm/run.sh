#!/bin/bash
#SBATCH --partition=batch
#SBATCH --ntasks-per-node=1
#SBATCH --nodes=1
#SBATCH --time=0:30:00

k = (10 25 50 75 100)

for i in ${k[@]}; do
    sbatch batch_link1.sh $i
    sbatch batch_link2.sh $i
    sbatch batch_link3.sh $i
    sbatch batch_link4.sh $i
done
