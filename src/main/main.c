/**
 * 1. master
 * 2. dataserver
 * 3. 
 */
#include <stdio.h>
#include <mpi.h>
#include "master.h"
#include "../client/client.h"

int main(argc, argv)
	int argc;char ** argv; {
	int rank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (rank == 0) {
		puts("========master start========");
		master_init();
	} else if (rank == 1) {
		puts("========client start========");
		client_init();
	}

	return MPI_Finalize();
}
