#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../dataserver.h"
#include "../../../common/communication/rpc_client.h"
#include "../../../common/communication/mpi_communication.h"
#include "../../../common/structure_tool/log.h"

void init_w_struct(write_c_to_d_t* w_msg)
{
	w_msg->chunks_count = 1;
	w_msg->chunks_id_arr[0] = 256;
	w_msg->offset = 0;
	w_msg->operation_code = 0x01;
	w_msg->write_len = 16;
	w_msg->unique_tag = 13;
}

void init_data_structure(msg_data_t* data_msg)
{
	data_msg->len = 16;
	data_msg->offset = 0;
	data_msg->seqno = 0;
	data_msg->tail = 1;
	data_msg->data[0] = 0x3031323334353637;
	data_msg->data[1] = 0x3736353433323130;
}

void init_r_structure(read_c_to_d_t* r_msg)
{
	r_msg->operation_code = MSG_READ;
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
	msg_data_t data_msg;
	read_c_to_d_t r_msg;

	log_init("", LOG_DEBUG);
	mpi_init_with_thread(&argc, &argv);
	id = get_mpi_rank();

	if(!id)
	{
		dataserver = alloc_dataserver(MIDDLE, 0);
		dataserver_run(dataserver);
	}
	else
	{
		rpc_client_t* client = create_rpc_client(id, 0, 13);
		//It can write to data server
		init_w_struct(&w_msg);
		printf("sending message to data server\n");
		client->op->set_send_buff(client, &w_msg);
		init_data_structure(&data_msg);
		client->op->set_second_send_buff(client, &data_msg, sizeof(data_msg));
		if(client->op->execute(client, WRITE) < 0)
			log_write(LOG_ERR, "client write wrong");
		destroy_rpc_client(client);

		//I will test read function
		client = create_rpc_client(id, 0, 13);
		init_r_structure(&r_msg);
		client->op->set_send_buff(client, &r_msg);
		memset(&data_msg, 0, sizeof(data_msg));
		client->op->set_recv_buff(client, &data_msg, sizeof(msg_data_t));
		if(client->op->execute(client, READ));
			log_write(LOG_ERR, "client read wrong");
		//printf_msg_status(&status);
		for(i = 0 ; i < 16; i++)
			printf("%c ", *((char*)data_msg.data + i));;
	}
	mpi_finish();
	return 0;
}
