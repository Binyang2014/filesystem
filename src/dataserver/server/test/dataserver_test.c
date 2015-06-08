#include <stdio.h>
#include <pthread.h>
#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../dataserver.h"

void init_w_struct(msg_w_ctod_t* w_msg)
{
	w_msg->chunks_count = 1;
	w_msg->chunks_id_arr[0] = 256;
	w_msg->offset = 0;
	w_msg->operation_code = 0x01;
	w_msg->write_len = 16;
	w_msg->unique_tag = 13;
}

void init_acc_struct(msg_acc_candd_t* acc_msg)
{
	acc_msg->operation_code = MSG_ACC;
	acc_msg->status = 0;
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

void init_r_structure(msg_r_ctod_t* r_msg)
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
	int id, p, provided;
	int i;
	data_server_t* dataserver;
	common_msg_t msg;
	msg_w_ctod_t w_msg;
	msg_acc_candd_t acc_msg;
	msg_data_t data_msg;
	msg_r_ctod_t r_msg;
	MPI_Status status;
	mpi_status_t mpi_status;

	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	if(!id)
	{
		dataserver = alloc_dataserver(MIDDLE, 0);
		dataserver_run(dataserver);
	}
	else
	{
		//It can write to data server
		init_w_struct(&w_msg);
		printf("sending message to data server\n");
		MPI_Send(&w_msg, MAX_CMD_MSG_LEN, MPI_CHAR, 0, D_MSG_CMD_TAG, MPI_COMM_WORLD);
		c_mpi_acc_recv(&(msg.operation_code), 0, 13, &mpi_status);
		init_acc_struct(&acc_msg);
		init_data_structure(&data_msg);
		MPI_Send(&acc_msg, MAX_CMD_MSG_LEN, MPI_CHAR, 0, 13, MPI_COMM_WORLD);
		MPI_Send(&data_msg, MAX_DATA_MSG_LEN, MPI_CHAR, 0, 13, MPI_COMM_WORLD);
		sleep(1);

		//I will test read function
		init_r_structure(&r_msg);
		MPI_Send(&r_msg, MAX_CMD_MSG_LEN, MPI_CHAR, 0, D_MSG_CMD_TAG, MPI_COMM_WORLD);
		MPI_Send(&acc_msg, MAX_CMD_MSG_LEN, MPI_CHAR, 0, 13, MPI_COMM_WORLD);
		memset(&data_msg, 0, sizeof(data_msg));
		MPI_Recv(&data_msg, MAX_DATA_MSG_LEN, MPI_CHAR, 0, 13, MPI_COMM_WORLD, &status);
		//printf_msg_status(&status);
		for(i = 0 ; i < 16; i++)
			printf("%c ", *((char*)data_msg.data + i));;
		sleep(2);
	}
	MPI_Finalize();
	return 0;
}
