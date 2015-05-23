mpicc -o filesystem main.c ../lib/libmaster.a ../lib/libclient.a ../lib/libdataserver.a ../lib/libtool.a \
../lib/libstruct.a ../lib/libvfs.a -lm -lpthread -g -O0
