#!/bin/bash -l
#SBATCH -J stencil_weak_icc
#SBATCH -p nodes
#SBATCH -N 2
#SBATCH --ntasks=32
#SBATCH --cpus-per-task=1
#SBATCH -t 1
#SBATCH -D ./
#SBATCH --output=weak_icc_%j.out
#SBATCH --error=weak_icc_%j.err
#SBATCH --export=ALL

module purge
module load compilers/intel/2019u5
module load mpi/intel-mpi/2019u5

make clean
make icccomplete           

DATADIR=linked_datafiles
FILTER=${DATADIR}/filter_5.dat
export OMP_NUM_THREADS=1

echo "===== Intel weak-scaling (OMP=1) ====="
for P in 1 2 4 8 16 32; do
    INPUT=${DATADIR}/input_${P}_3840_2048.dat
    echo "-> MPI ranks = $P"
    srun --ntasks=$P --cpus-per-task=1 ./stencil-complete-icc "$INPUT" "$FILTER" "out_icc_${P}r.dat"
done
