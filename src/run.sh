#!/bin/bash

#SBATCH --time=30

mpirun --map-by ppr:1:node ./main 10000 10000 50 100
