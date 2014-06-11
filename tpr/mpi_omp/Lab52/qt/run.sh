#!/bin/bash

for i in 1 2 3 4 5 6 7 8
do
	echo "nodes $i"
	mpiexec -machinefile ./mpihosts -np $i ./a.out
done
