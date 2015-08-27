#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../dataserver.h"
#include "../../../common/communication/rpc_client.h"
#include "../../../common/communication/mpi_communication.h"
#include "../../../common/structure_tool/log.h"
#include "../../../common/structure_tool/zmalloc.h"

void init_w_struct(write_c_to_d_t* w_msg)
{
	w_msg->chunks_count = 1;
	w_msg->chunks_id_arr[0] = 256;
	w_msg->offset = 0;
	w_msg->operation_code = C_D_WRITE_BLOCK_CODE;
	w_msg->write_len = 16;
	w_msg->unique_tag = 13;
}

void init_data_structure(char data_msg[])
{
	int i;
	for(i = 0; i < 16; i++)
		data_msg[i] = '0' + i;
}

void init_r_structure(read_c_to_d_t* r_msg)
{
	r_msg->operation_code = C_D_READ_BLOCK_CODE;
	r_msg->chunks_id_arr[0] = 256;
	r_msg->offset = 0;
	r_msg->read_len = 16;
	r_msg->unique_tag = 13;
	r_msg->chunks_count = 1;
}

int main(int argc, char* argv[])
{
	int id;
	int i;
	data_server_t* dataserver;
	write_c_to_d_t w_msg;
	read_c_to_d_t r_msg;
	char data_msg[16];

	log_init("", LOG_DEBUG);
	mpi_init_with_thread(&argc, &argv);
	id = get_mpi_rank();

	if(!id)
	{
		dataserver = alloc_dataserver(MIDDLE, 0);
		dataserver_run(dataserver);
		destroy_dataserver(dataserver);
	}
	else
	{
		stop_server_msg_t* stop_server_msg = NULL;
		rpc_client_t* client = create_rpc_client(id, 0, 13);
		//It can write to data server
		init_w_struct(&w_msg);
		printf("sending message to data server\n");
		client->op->set_send_buff(client, &w_msg);
		init_data_structure(data_msg);
		client->op->set_second_send_buff(client, data_msg, 16);
		if(client->op->execute(client, WRITE_C_TO_D) < 0)
			log_write(LOG_ERR, "client write wrong");
		destroy_rpc_client(client);

		//I will test read function
		client = create_rpc_client(id, 0, 13);
		init_r_structure(&r_msg);
		client->op->set_send_buff(client, &r_msg);
		memset(data_msg, 0, sizeof(data_msg));
		client->op->set_recv_buff(client, data_msg, 16);
		if(client->op->execute(client, READ_C_TO_D) < 0)
			log_write(LOG_ERR, "client read wrong");
		for(i = 0 ; i < 16; i++)
			printf("%c ", data_msg[i]);
		printf("\n");
		destroy_rpc_client(client);

		//send message to stop data server
		client = create_rpc_client(id, 0, 13);
		stop_server_msg = zmalloc(sizeof(stop_server_msg_t));
		stop_server_msg->operation_code = SERVER_STOP;
		stop_server_msg->source = 1;
		stop_server_msg->tag = 13;
		client->op->set_send_buff(client, stop_server_msg);
		if(client->op->execute(client, STOP_SERVER) < 0)
			log_write(LOG_ERR, "stop server wrong");
		else
			log_write(LOG_INFO, "can't believe it");
		destroy_rpc_client(client);
		zfree(stop_server_msg);
	}
	mpi_finish();
	return 0;
}
