mpicc -o rpc mpi_rpc_server.c mpi_rpc_client.c ../zmalloc.c ../sds.c ../syn_tool.c ../basic_list.c ../threadpool.c ../basic_queue.c ../log.c -lm -g
