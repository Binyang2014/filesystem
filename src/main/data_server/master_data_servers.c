/*
 * data_server.c
 *
 *  Created on: 2015年5月12日
 *      Author: ron
 */

#include <stdio.h>
#include "master_data_servers.h"
#include "../../structure/basic_list.h"

/*===============private functions===============*/
static double load_situation(master_data_server *);
static void free_file_machine_location(file_machine_location *);

/*================private functions implementation===============*/
static double load_situation(master_data_server *server){
	return (double)server->used_block/(server->used_block + server->free_block);
}

static void free_file_machine_location(file_machine_location *location){
	free(location);
}

static int heart_bleed(data_servers *servers, int server_id){
	if(server_id > servers->servers_count)
		return -1;
	//TODO here we can use a function caches a time_t value just like redis
	time(&(*(servers->server_list + server_id)->last_update));
	return 0;
}

list_t *file_allocate_machine(data_servers *servers, unsigned long file_size, int block_size){
	int block_num = ceil(file_size / block_size);
	list_t *list = list_create();
	list_set_free_method(list, free_file_machine_location);
	int seq = 0;
	int count = 0;
	int server_index = 0;
	master_data_server *ptr;
	for(server_index = 0; server_index != servers->servers_count; server_index++){
		ptr = servers->server_list + server_index;
		//TODO ptr->status
		if(ptr->free_block == 0 || ptr->status){
			continue;
		}

		if(ptr->free_block > block_num){
			count = block_num;
			block_num = 0;
			ptr->free_block -= block_num;
		}else{
			count = ptr->free_block;
			block_num -= count;
			ptr->free_block = 0;
		}
		file_machine_location* location = create_file_machine_location(count, ptr->server_id, seq, servers->global_id);
		servers->global_id += count;
		list_add_node_tail(list, location);
	}

	if(block_num == 0){
		return list;
	}
	else{
		list_iter_t *tmp = list_get_iterator(list, AL_START_HEAD);
		while(tmp){
			file_machine_location* location = ((file_machine_location*)(tmp->next->value));
			servers->global_id -= location->count;
			(servers->server_list + location->machinde_id)->free_block = location->count;
			tmp = list_next(tmp);
			//file_machine_location
		}
		listRelease(list);
		return NULL;
	}
}

data_servers *data_servers_create(int server_count, double load_factor){
	data_servers *servers = (data_servers *)malloc(sizeof(data_servers));
	if(servers == NULL){
		return NULL;
	}

	servers->server_list = (master_data_server*)malloc(sizeof(master_data_server) * server_count);
	if(servers->server_list == NULL){
		free(servers);
		return NULL;
	}

	servers->global_id = 0;
	servers->load_factor = load_factor;
	servers->opera->heart_blood = heart_blood;
	servers->servers_count = server_count;
	//servers->
	return servers;
}


