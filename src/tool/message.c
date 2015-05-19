/**
 * create on: 2015.3.29
 * author: Binyang
 *
 * This file offer complete some basic message functions
 */
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include "message.h"

void printf_msg_status(mpi_status_t* status)
{
	char s[127];
	int len;
	printf("The information comes from MPI status\n");
	printf("The MPI source is %d\n", status->source);
	printf("The MPI tag is %d\n", status->tag);
	MPI_Error_string(status->error_num, s, &len);
	printf("The MPI error is %s\n", s);
}

//data server receive cmd message
void d_mpi_cmd_recv(void* msg, mpi_status_t* status_t)
{
	MPI_Status status;
	MPI_Recv(msg, MAX_CMD_MSG_LEN, MPI_CHAR, MPI_ANY_SOURCE,
						D_MSG_CMD_TAG, MPI_COMM_WORLD, &status);
	status_t->error_num = status.MPI_ERROR;
	status_t->size = status.count;
	status_t->source = status.MPI_SOURCE;
	status_t->tag = status.MPI_TAG;
}

//data server receive accept message
void d_mpi_acc_recv(void* msg, int source, int tag, mpi_status_t* status_t)
{
	MPI_Status status;
	MPI_Recv(msg, MAX_CMD_MSG_LEN, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
	status_t->error_num = status.MPI_ERROR;
	status_t->size = status.count;
	status_t->source = status.MPI_SOURCE;
	status_t->tag = status.MPI_TAG;
}

//data receive data message
void d_mpi_data_recv(void* msg, int source, int tag, mpi_status_t* status_t)
{
	MPI_Status status;
	MPI_Recv(msg, MAX_DATA_MSG_LEN, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
	status_t->error_num = status.MPI_ERROR;
	status_t->size = status.count;
	status_t->source = status.MPI_SOURCE;
	status_t->tag = status.MPI_TAG;
}

void d_mpi_data_send(void* msg, int source, int tag)
{
	MPI_Send(msg, MAX_DATA_MSG_LEN, MPI_CHAR, source, tag, MPI_COMM_WORLD);
}


void common_msg_dup(void *dest, void *source){
	memcpy(dest, source, sizeof(common_msg_t));
}

void common_msg_free(void *msg){
	free(msg);
}
