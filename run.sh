mv src/fileapp . | exit 1
mpirun -f mpifile ./fileapp topo.conf
