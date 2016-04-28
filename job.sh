#!/bin/bash
#PBS -j oe -m ae
#PBS -l nodes=32:ppn=1
#PBS -l walltime=30:00
#PBS -l mem=8GB
set -eu
module add GNU/4.9 OpenMPI/1.10.0
cd "${PBS_O_WORKDIR-.}"
for m in 25 50 100 200 300 400 500 600 700 800 900 1000; do
    for n in 1 2 4 8 16 32; do
        for x in $(seq 1 10); do
            printf "@ $m $n"
            mpiexec -n "$n" src/phy905proj "$m"
        done
    done
done
