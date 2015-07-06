/**
 * create on: 2015.3.29
 * author: Binyang
 *
 * This file offer complete some basic message functions
 */
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "message.h"

void printf_msg_status(mpi_status_t* status)
{
	assert(status != NULL);

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
	assert(msg != NULL && status_t != NULL);

	MPI_Status status;
	MPI_Recv(msg, MAX_CMD_MSG_LEN, MPI_CHAR, MPI_ANY_SOURCE, D_MSG_CMD_TAG, MPI_COMM_WORLD, &status);
	status_t->error_num = status.MPI_ERROR;
	//status_t->size = status.count;
	status_t->source = status.MPI_SOURCE;
	status_t->tag = status.MPI_TAG;
}

//data server receive accept message
void d_mpi_acc_recv(void* msg, int source, int tag, mpi_status_t* status_t)
{
	assert(msg != NULL && status_t != NULL);

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
	assert(msg != NULL && status_t != NULL);

	MPI_Status status;
	MPI_Recv(msg, MAX_DATA_MSG_LEN, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
	status_t->error_num = status.MPI_ERROR;
	status_t->size = status.count;
	status_t->source = status.MPI_SOURCE;
	status_t->tag = status.MPI_TAG;
}

void d_mpi_data_send(void* msg, int source, int tag)
{
	assert(msg != NULL);

	MPI_Send(msg, MAX_DATA_MSG_LEN, MPI_CHAR, source, tag, MPI_COMM_WORLD);
}

//data send cmd message
void d_mpi_cmd_send(void* msg, int source, int tag)
{
	assert(msg != NULL);

	MPI_Send(msg, MAX_CMD_MSG_LEN, MPI_CHAR, source, tag, MPI_COMM_WORLD);
}

//master send cmd message
void m_mpi_cmd_send(void *msg, int source, int tag)
{
	assert(msg != NULL);

	MPI_Send(msg, MAX_CMD_MSG_LEN, MPI_CHAR, source, tag, MPI_COMM_WORLD);
}

void m_mpi_cmd_recv(void *msg, mpi_status_t* status_t)
{
	assert(msg != NULL && status_t != NULL);
	MPI_Status status;
	MPI_Recv(msg, MAX_DATA_MSG_LEN, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	status_t->error_num = status.MPI_ERROR;
	status_t->size = status.count;
	status_t->source = status.MPI_SOURCE;
	status_t->tag = status.MPI_TAG;
}

void c_mpi_acc_recv(void* msg, int source, int tag, mpi_status_t* status_t)
{
	assert(msg != NULL && status_t != NULL);

	MPI_Status status;
	MPI_Recv(msg, MAX_CMD_MSG_LEN, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
	status_t->error_num = status.MPI_ERROR;
	status_t->size = status.count;
	status_t->source = status.MPI_SOURCE;
	status_t->tag = status.MPI_TAG;
}

void common_msg_dup(void *dest, void *source){
	memcpy(dest, source, sizeof(common_msg_t));
}
