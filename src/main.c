/**
 * 1. master
 * 2. dataserver
 * 3.
 */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <mpi.h>
#include "./client/client.h"
#include "./master/master.h"
#include "./dataserver/server/dataserver.h"
pthread_t p_server, p_client;

int main(argc, argv)
int argc;char ** argv;
{
	int rank, size, provided;
	data_server_t* dataserver;

	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {
		puts("========master start========");
		master_init(size - 1);
	} else {
		puts("========client start========");
		//sleep(3);
		dataserver = alloc_dataserver(MIDDLE, rank);
		pthread_create(&p_server, NULL, dataserver_run, (void *)dataserver);
		sleep(3);
		if(rank == 1){
			pthread_create(&p_client, NULL, client_init, NULL);
			pthread_join(p_client, NULL);
		}
		pthread_join(p_server, NULL);

	}
	MPI_Finalize();
	return 0;
}
