/*
 * data_server_test.c
 *
 *  Created on: 2015年5月15日
 *      Author: ron
 */
#include <stdio.h>

#include "../../master/data_server/master_data_servers.h"
#include "../../structure/basic_queue.h"
#include "../../tool/message.h"


void test_master_data_server(){
	data_servers *d = data_servers_create(1024, 0.75, 4);
	if(d == NULL){
		puts("allocate space failed");
	}else{
		puts("hahahah");
	}
	//printf("free block nums = %d\n", d->server_list->free_block);
	basic_queue_t *queue = d->opera->file_allocate_machine(d, 1000, 100);
	basic_queue_iterator *iterator = create_basic_queue_iterator(queue);
	printf("iterator->offset %d iterator->queue->tail_pos %d\n", iterator->offset, iterator->queue->tail_pos);
	block_location location;
	while(iterator->has_next(iterator)){
		iterator->next(iterator, &location);
		printf("server_id = %d, block_seq = %ld, global_id = %ld\n", location.server_id, location.block_seq, location.global_id);
	}
}

int main(){
	test_master_data_server();
	return 0;
}
