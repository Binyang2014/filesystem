#include <stdio.h>
#include <stdlib.h>
#include "../dataserver_buff.h"
#include "../dataserver.h"

int main()
{
	data_server_t* dataserver;
	list_t* list;
	list_node_t* node;
	list_iter_t* iter;
	void* value;
	int i;
	dataserver = (data_server_t*)malloc(sizeof(data_server_t));
	set_data_server_buff(dataserver, 8);
	printf("The node queue size is %d\n", dataserver->buff_node_queue->current_size);
	list = get_buffer_list(dataserver, 5);

	//test buff_node_queue
	iter = list->list_ops->list_get_iterator(list, 0);
	for(i = 0; i < 5; i++)
	{
		node = list->list_ops->list_next(iter);
		printf("the address of the node is %p\n", node);
		printf("the value of the node is %p, the next is %p, the prev is %p\n",
				node->value, node->next, node->prev);
	}
	printf("The node queue size is %d\n", dataserver->buff_node_queue->current_size);

	list->list_ops->list_rewind(list, iter);
	//test common_msg_buff
	node  = list->list_ops->list_next(iter);
	value = get_common_msg_buff(dataserver);
	list_set_node_value(node, value);
	printf("the common message operation code is %d\n", ((common_msg_t*)value)->operation_code);
	reture_common_msg_buff(dataserver, value);

	//test common_msg_buff
	node  = list->list_ops->list_next(iter);
	value = get_data_buff(dataserver);
	list_set_node_value(node, value);
	return_data_buff(dataserver, value);

	//test file_info_buff
	node  = list->list_ops->list_next(iter);
	value = get_file_info_buff(dataserver);
	list_set_node_value(node, value);
	return_file_info_buff(dataserver, value);

	//test reply_message_buff
	//node  = list->list_ops->list_next(iter);
	//value = get_reply_msg_buff(dataserver);
	//list_set_node_value(node, value);
	//return_reply_msg_buff(dataserver, value);

	//test f_arr_buff
	node  = list->list_ops->list_next(iter);
	value = get_f_arr_buff(dataserver, 5);
	list_set_node_value(node, value);
	printf("f_arr_buff size is %d\n", ((vfs_hashtable_t*)value)->hash_table_size);
	return_f_arr_buff(dataserver, value);

	//finish test
	list->list_ops->list_release_iterator(iter);
	return_buffer_list(dataserver, list);
	printf("The node queue size is %d\n", dataserver->buff_node_queue->current_size);
	free_data_server_buff(dataserver);
	free(dataserver);
	return 0;
}
