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

int main_init(char *file_name){

}

int main(argc, argv)
	int argc;char ** argv; {
	int rank, size;
	pthread_t *thread_data_master, *thread_data_server, *thread_client;

	mpi_init_with_thread(&argc, &argv);
	rank = get_mpi_rank();
	size = get_mpi_size();

	map_role_value_t *map_role = NULL;
	char *file_path = "topo.conf";
	char *net_name = "eth0";

	log_init("", LOG_TRACE);
	log_write(LOG_DEBUG, "file open successfully");

	if (rank == 0) {
		map_role = machine_role_allocator_start(size, rank, file_path, net_name);
	}else{
		usleep(50);
		map_role = get_role(rank, net_name);
	}
	usleep(10);
	log_write(LOG_DEBUG, "get role success and role id is %d rol ip is %s ", map_role->rank, map_role->ip);

	thread_data_master = (pthread_t *)zmalloc(sizeof(*thread_data_master));
	thread_data_server = (pthread_t *)zmalloc(sizeof(*thread_data_server));
	thread_client = (pthread_t *)zmalloc(sizeof(*thread_client));

	if (map_role->type == DATA_MASTER) {
		data_master_t *master = create_data_master(map_role, 1024);
		data_server_t *server = alloc_dataserver(LARGEST, rank);
		fclient_t *fclient = create_fclient(rank, map_role->master_rank, CLIENT_LISTEN_TAG);

//		pthread_create(thread_data_master, NULL, data_master_init, master);
//		pthread_create(thread_data_server, NULL, dataserver_run, server);
//		pthread_create(thread_client, NULL, fclient_run, fclient);
//		pthread_join(*thread_data_master, NULL);
//		pthread_join(*thread_data_server, NULL);
		pthread_join(*thread_client, NULL);
	} else if (map_role->type == DATA_SERVER) {
		data_server_t *server = alloc_dataserver(SMALL, rank);
//		fclient_t *fclient = create_fclient(rank, map_role->master_rank, CLIENT_LISTEN_TAG);

//		pthread_create(thread_data_server, NULL, dataserver_run, server);
//		pthread_create(thread_client, NULL, fclient_run, fclient);
//		pthread_join(*thread_data_server, NULL);
//		pthread_join(*thread_client, NULL);
	}

	mpi_finish();
	return 0;
}


