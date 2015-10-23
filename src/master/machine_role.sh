mpicc -o role machine_role.c ../common/structure_tool/zmalloc.c \
../common/structure_tool/threadpool.c ../common/structure_tool/sds.c \
../common/structure_tool/basic_list.c ../common/structure_tool/map.c \
../common/structure_tool/basic_queue.c ../common/structure_tool/log.c \
../common/communication/rpc_server.c ../common/structure_tool/syn_tool.c \
../common/communication/mpi_communication.c ../common/communication/message.c
