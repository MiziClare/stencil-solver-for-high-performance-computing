#!/bin/bash -l
#SBATCH -J stencil_strong_icc
#SBATCH -p nodes
#SBATCH -N 1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH -t 1
#SBATCH -D ./
#SBATCH --output=strong_icc_%j.out
#SBATCH --error=strong_icc_%j.err
#SBATCH --export=ALL

module purge
module load compilers/intel/2019u5
module load mpi/intel-mpi/2019u5

make clean
make iccnearly             

DATADIR=linked_datafiles
INPUT=${DATADIR}/input_1_3840_2048.dat
FILTER=${DATADIR}/filter_5.dat

echo "===== Intel OpenMP strong-scaling (1 MPI rank) ====="
for T in 1 2 4 8 16 32; do
    export OMP_NUM_THREADS=$T
    echo "-> Threads = $T"
    srun --cpus-per-task=$T ./stencil-nearly-icc "$INPUT" "$FILTER" "out_icc_${T}.dat"
done
