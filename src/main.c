/**
 * 1. master
 * 2. dataserver
 * 3.
 */
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include "./client/client.h"
#include "./master/master.h"
#include "./dataserver/server/dataserver.h"

int main(argc, argv)
int argc;char ** argv;
{
	MPI_Init(&argc, &argv);
	int rank, size;
	data_server_t* dataserver;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {
		puts("========master start========");
		master_init();
	} else if (rank == 1) {
		puts("========client start========");
		//sleep(3);
		dataserver = alloc_dataserver(MIDDLE, 1);
		dataserver_run(dataserver);
		client_init();
	}

	MPI_Finalize();
	return 0;
}
