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
#include "client_server.h"
#include "log.h"
#include "machine_role.h"
#include "data_master.h"
#include "dataserver.h"
#include "client.h"
#include "message.h"
#include "zmalloc.h"

int main(argc, argv)
	int argc;char ** argv; {
	int rank, size, provided;
	pthread_t *thread_data_master, *thread_data_server, *thread_client;

	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
	{
		char *file_path = "";
		machine_role_allocator_start(size, 0, file_path);
	}

	thread_data_master = (pthread_t *)zmalloc(sizeof(*thread_data_master));
	thread_data_server = (pthread_t *)zmalloc(sizeof(*thread_data_server));
	thread_client = (pthread_t *)zmalloc(sizeof(*thread_client));

	map_role_value_t *role = get_role(rank, "eth0");
	if (role->type == DATA_MASTER) {
		data_master_t *master = create_data_master(role);
		data_server_t *server = alloc_dataserver(LARGEST, rank);
		//TODO tag
		fclient_t *fclient = create_fclient(rank, role->master_rank, CLIENT_LISTEN_TAG);

		pthread_create(thread_data_master, NULL, data_master_init, master);
		pthread_create(thread_data_server, NULL, dataserver_run, server);
		pthread_create(thread_client, NULL, fclient_run, fclient);
		pthread_join(*thread_data_master, NULL);
		pthread_join(*thread_data_server, NULL);
		pthread_join(*thread_client, NULL);
	} else if (role->type == DATA_SERVER) {
		data_server_t *server = alloc_dataserver(LARGEST, rank);
		//TODO tag
		fclient_t *fclient = create_fclient(rank, role->master_rank, CLIENT_LISTEN_TAG);

		pthread_create(thread_data_server, NULL, dataserver_run, server);
		pthread_create(thread_client, NULL, fclient_run, fclient);
		pthread_join(*thread_data_server, NULL);
		pthread_join(*thread_client, NULL);
	}

	MPI_Finalize();
	return 0;
}
