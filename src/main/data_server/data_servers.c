/*
 * data_server.c
 *
 *  Created on: 2015年5月12日
 *      Author: ron
 */

#include "data_servers.h"
#include <stdio.h>


static int heart_bleed(data_servers *servers, int server_id){

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


