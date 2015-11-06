inc="-Isrc -Isrc/common/structure_tool -Isrc/common/communication 
-Isrc/common/zookeeper -Isrc/common/ipc -Isrc/data-server/structure
-Isrc/data-master -Isrc/master -Isrc/data-server/server -Isrc/client"
ld="./lib"
lib="${ld}/libcommon.a ${ld}/libclient.a ${ld}/libdataserver.a ${ld}/libdatamaster.a ${ld}/libmachinerole.a ${ld}/libvfs.a"
depend_order="-lcommon -lclient -ldataserver -ldatamaster -lmachinerole -lvfs"
mpicc -o filesystem ./src/main.c ${inc} ${lib} -Wall -lm -lpthread -g -O0
