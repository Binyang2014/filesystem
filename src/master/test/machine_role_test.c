/*
 * machine_role_test.c
 *
 *  Created on: 2015年10月28日
 *      Author: ron
 */

#include <mpi.h>
#include "machine_role.h"
#include "zmalloc.h"
#include "log.h"

int main(argc, argv)
	int argc;char ** argv; {
	int rank, size, provided;

	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	log_init("", LOG_DEBUG);
	log_write(LOG_DEBUG, "file open successfully");
	if (rank == 0) {
		char *file_path = "topo.conf";
		puts("start");
		machine_role_allocator_start(size, rank, file_path);
	}

	usleep(50);
	map_role_value_t *role = get_role(rank, "eth0");
	zfree(role);

	log_write(LOG_DEBUG, "file open successfully");
	MPI_Finalize();
	return 0;
}
