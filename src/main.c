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
#include "data_master_request.h"
#include "mpi_communication.h"
#include "user_func.h"
#include "test_conf.h"

static c_d_register_t* get_register_cmd(int rank, uint64_t freeblock, int tag, char *net_name)
{
	c_d_register_t *re = zmalloc(sizeof(*re));
	re->free_block =  (1 << freeblock) / BLOCK_SIZE - 8;
	re->source = rank;
	//get_visual_ip(net_name, re->ip);
	re->operation_code = REGISTER_TO_DATA_MASTER_CODE;
	re->unique_tag = tag;
	return re;
}

int main(argc, argv)
	int argc;char ** argv;
{
	int data_master_free_blocks = 0;
	int data_server_free_blocks = MEMORY_SIZE;
	int rank;
	int size;
//	char *file_path = "topo.conf";
	char *net_name = "eth0";
	pthread_t *thread_data_master = (pthread_t *)zmalloc(sizeof(*thread_data_master));
	pthread_t *thread_data_server = (pthread_t *)zmalloc(sizeof(*thread_data_master));
	pthread_t *thread_client = (pthread_t *)zmalloc(sizeof(*thread_data_master));
	map_role_value_t *map_role =  zmalloc(sizeof(*map_role));
	mpi_init_with_thread(&argc, &argv);
	rank = get_mpi_rank();
	size = get_mpi_size();

	log_init("", LOG_ERR);
	log_write(LOG_DEBUG, "get role success and role id is %d role ip is %s ", map_role->rank, map_role->ip);

	if (rank == 0)
	{
		map_role->type = DATA_MASTER;
		map_role->master_rank = 0;
		map_role->group_size = size - 1;
		map_role->rank = 0;
		log_write(LOG_DEBUG, "ROLE DATA_MASTER AND ID IS %d", rank);
		data_master_t *master = create_data_master(map_role, data_master_free_blocks);
		//fclient_t *fclient = create_fclient(rank, map_role->master_rank, CLIENT_LISTEN_TAG);

		pthread_create(thread_data_master, NULL, data_master_init, master);
		//pthread_create(thread_client, NULL, fclient_run, fclient);
		pthread_join(*thread_data_master, NULL);
//		pthread_join(*thread_client, NULL);
	} else
	{
		if(rank %2 == 1)
		{
			map_role->type = DATA_SERVER;
			map_role->master_rank = 0;
			map_role->group_size = size - 1;
			map_role->rank = rank;
			log_write(LOG_DEBUG, "ROLE DATA_SERVER and ID IS %d and master rank = %d", rank, map_role->master_rank);
			usleep(10);
			//register to master
			c_d_register_t* re = get_register_cmd(rank, data_server_free_blocks, rank, net_name);
			data_master_request_t *request = create_data_master_request(rank, map_role->master_rank, 169 + rank);
			request->op->register_to_master(request, re);
			destroy_data_master_request(request);
			zfree(re);

			data_server_t *server = alloc_dataserver(data_server_free_blocks, rank);
			//fclient_t *fclient = create_fclient(rank, map_role->master_rank, CLIENT_LISTEN_TAG);
			pthread_create(thread_data_server, NULL, dataserver_run, server);

			//pthread_create(thread_client, NULL, fclient_run, fclient);
			pthread_join(*thread_data_server, NULL);
			//pthread_join(*thread_client, NULL);
		}else
		{
			fclient_t *fclient = create_fclient(rank, map_role->master_rank, CLIENT_LISTEN_TAG);
			fclient_run(fclient);
		}
	}

	mpi_finish();
	return 0;
}


