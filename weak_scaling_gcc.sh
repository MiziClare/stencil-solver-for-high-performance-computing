#!/bin/bash -l
#SBATCH -J stencil_weak_gcc
#SBATCH -p nodes
#SBATCH -N 2               
#SBATCH --ntasks=32        
#SBATCH --cpus-per-task=1  
#SBATCH -t 1
#SBATCH -D ./
#SBATCH --output=weak_gcc_%j.out
#SBATCH --error=weak_gcc_%j.err
#SBATCH --export=ALL

module purge
module load apps/cgns/3.3.0/gcc-9.3.0+openmpi-3.1.6+hdf5_mpi-1.10.5

make clean
make gcccomplete            

DATADIR=linked_datafiles
FILTER=${DATADIR}/filter_5.dat
export OMP_NUM_THREADS=1    

echo "===== GCC weak-scaling (OMP=1) ====="
for P in 1 2 4 8 16 32; do
    INPUT=${DATADIR}/input_${P}_3840_2048.dat   # b = P
    echo "-> MPI ranks = $P"
    srun --ntasks=$P --cpus-per-task=1 ./stencil-complete-gcc "$INPUT" "$FILTER" "out_gcc_${P}r.dat"
done
