/*
 * data_server.c
 *
 *  Created on: 2015年5月12日
 *      Author: ron
 */

#include <stdio.h>
#include "master_data_servers.h"


static int heart_bleed(data_servers *servers, int server_id){
	if(server_id > servers->server_count)
		return -1;
	//TODO here we can use a function caches a time_t value just like redis
	time(&(*(servers->server_list + server_id)->last_update));
	return 0;
}

static file_location_des *file_location_allocate(data_servers *servers, unsigned long file_size, int block_size){
	file_location_des *location = create_file_location_des();
	if(location == NULL)
		return NULL;


	return location;
}

data_servers *data_servers_create(int server_count, double load_factor){
	data_servers *servers = (data_servers *)malloc(sizeof(data_servers));
	if(servers == NULL){
		return NULL;
	}

	servers->data_server_list = (master_data_server*)malloc(sizeof(master_data_server) * server_count);
	if(servers->data_server_list == NULL){
		free(servers);
		return NULL;
	}

	servers->global_id = 0;
	servers->load_factor = load_factor;
	servers->opera->heart_blood = heart_blood;
	servers->server_count = server_count;
	//servers->
}


