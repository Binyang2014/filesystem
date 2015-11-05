/*
 * main.c
 *
 *  Created on: 2015年7月6日
 *      Author: ron
 */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <mpi.h>
#include "master/machine_role.h"
#include "data-master/data_master.h"

int main(argc, argv)
	int argc;char ** argv; {
	int rank, size, provided;
	data_server_t* dataserver;

	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);


	if (rank == 0) {
		char *file_path = "";
		machine_role_allocator_start(size, 0, file_path);
	} else {
		map_role_value_t *role = get_role(rank);
		if (role->type == DATA_MASTER) {
			data_master_t *master = create_data_master(role);
			data_master_init(master);
			//client
		}else if(role->type == DATA_SERVER){
			//init data server
		}

		//register to data  master
		//listen to the request
	}
	MPI_Finalize();
	return 0;
}
