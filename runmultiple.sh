#!/bin/sh

#  runmultiple.sh
#  
#
#  Created by Shahrzad Tighnavard on 2015-11-30.
#  
ARRAY=( 10 100 1000 10000 100000 1000000 )
ELEMENTS=${#ARRAY[@]}

make clean
make

echo "****************************\n" >> runinfo.txt
echo "👾 I guess I'm starting your mandelbrot set programs now at" >> runinfo.txt
date >> runinfo.txt
echo "****************************\n" >> runinfo.txt
for(( i=0;i<2;i++)); do
    echo "Runnng the " ${i} "th"
    for (( j=0;j<$ELEMENTS;j++)); do
        ./mandelbrot_seq ${ARRAY[${j}]} 0 0 2
        ./rows_omp ${ARRAY[${j}]} 2 0 0 2
        ./rows_omp ${ARRAY[${j}]} 4 0 0 2
        ./rows_omp ${ARRAY[${j}]} 8 0 0 2
        ./points_omp ${ARRAY[${j}]} 2 0 0 2
        ./points_omp ${ARRAY[${j}]} 4 0 0 2
        ./points_omp ${ARRAY[${j}]} 8 0 0 2
        mpirun -np 2 ./block_mpi ${ARRAY[${j}]} 0 0 2
        mpirun -np 4 ./block_mpi ${ARRAY[${j}]} 0 0 2
        mpirun -np 8 ./block_mpi ${ARRAY[${j}]} 0 0 2
        mpirun -np 2 ./cyclic_mpi ${ARRAY[${j}]} 0 0 2
        mpirun -np 4 ./cyclic_mpi ${ARRAY[${j}]} 0 0 2
        mpirun -np 8 ./cyclic_mpi ${ARRAY[${j}]} 0 0 2
        mpirun -np 2 ./dynamic_mpi ${ARRAY[${j}]} 0 0 2
        mpirun -np 4 ./dynamic_mpi ${ARRAY[${j}]} 0 0 2
        mpirun -np 8 ./dynamic_mpi ${ARRAY[${j}]} 0 0 2
    done
done
echo "****************************\n" >> runinfo.txt
echo "👾done all the work you asked me :D" >> runinfo.txt
date >> runinfo.txt
echo "****************************\n" >> runinfo.txt