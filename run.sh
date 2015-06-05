ld="./lib"
depend_order="${ld}/libmaster.a ${ld}/libclient.a ${ld}/libdataserver.a ${ld}/libtool.a ${ld}/libstruct.a ${ld}/libvfs.a"
mpicc -o filesystem ./src/main.c ${depend_order} -lm -lpthread -g -O0
#mpirun -np 2 ./filesystem
