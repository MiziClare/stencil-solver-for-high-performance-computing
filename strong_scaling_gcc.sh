#!/bin/bash -l
#SBATCH -J stencil_strong_gcc
#SBATCH -p nodes
#SBATCH -N 1               
#SBATCH --ntasks=1         
#SBATCH --cpus-per-task=32 
#SBATCH -t 1               
#SBATCH -D ./
#SBATCH --output=strong_gcc_%j.out
#SBATCH --error=strong_gcc_%j.err
#SBATCH --export=ALL

module purge
module load apps/cgns/3.3.0/gcc-9.3.0+openmpi-3.1.6+hdf5_mpi-1.10.5

make clean
make gccnearly               

DATADIR=linked_datafiles
INPUT=${DATADIR}/input_1_3840_2048.dat   # b = 1
FILTER=${DATADIR}/filter_5.dat

echo "===== GCC OpenMP strong-scaling (1 MPI rank) ====="
for T in 1 2 4 8 16 32; do
    export OMP_NUM_THREADS=$T
    echo "-> Threads = $T"
    srun --cpus-per-task=$T ./stencil-nearly-gcc "$INPUT" "$FILTER" "out_gcc_${T}.dat"
done
