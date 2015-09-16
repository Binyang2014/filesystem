/*
 * mpi_communication.c
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 */
#include <stdio.h>
#include <assert.h>
#include <mpi.h>
#include <stdlib.h>
#include "log.h"
#include "mpi_communication.h"

static void printf_msg_status(mpi_status_t* status)
{
	assert(status != NULL);
	char s[512];
	int len;

	MPI_Error_string(status->error_num, s, &len);
	log_write(LOG_DEBUG, "The information comes from MPI status The MPI source is %d, The MPI tag is %d The MPI error is %s",
			status->source, status->tag, s);
}

void mpi_send(void* msg, int dst, int tag, int len)
{
	assert(msg != NULL);
	int source = get_mpi_rank();

	if(dst < 0)
	{
		log_write(LOG_ERR, "destination is wrong when sending message");
		return;
	}
	log_write(LOG_INFO, "sending message to receiver %d from sender %d the length is %d", dst, source, len);
	MPI_Send(msg, len, MPI_CHAR, dst, tag, MPI_COMM_WORLD);
}

//receiving message from sender and if souce  equal to -1 it will receiving
//message from any souce, the same to tag
void mpi_recv(void* msg, int source, int tag, int len, mpi_status_t* status_t)
{
	assert(msg != NULL);
	int dst = get_mpi_rank();

	MPI_Status status;
	//-2 means MPI_ANY_SOURCE
	if(source < -2)
	{
		log_write(LOG_ERR, "destination is wrong when sending message");
		return;
	}
	if(source == -1)
		source = MPI_ANY_SOURCE;
	if(tag == -1)
		tag = MPI_ANY_TAG;

	MPI_Recv(msg, len, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
	log_write(LOG_INFO, "received message from sender %d and the receiver is %d the length is %d",
			status.MPI_SOURCE, dst, status.count);
	if(status_t != NULL)
	{
		status_t->error_num = status.MPI_ERROR;
		status_t->size = status.count;
		status_t->source = status.MPI_SOURCE;
		status_t->tag = status.MPI_TAG;
#ifdef MPI_COMMUNICATION_DEBUG
		printf_msg_status(status_t);
#endif
	}
}

void mpi_server_recv(void* msg, int len, mpi_status_t* status) {
	mpi_recv(msg, MPI_ANY_SOURCE, MPI_ANY_TAG, len, status);
}

int get_mpi_size()
{
	int pro_num;
	MPI_Comm_size(MPI_COMM_WORLD, &pro_num);
	return pro_num;
}

int get_mpi_rank()
{
	int id;
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	return id;
}

void mpi_init_with_thread(int* argc, char ***argv)
{
	int provided;

	MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
	if(provided != MPI_THREAD_MULTIPLE)
	{
		log_write(LOG_ERR, "could not support multiple threads in mpi process");
		exit(1);
	}
}
void mpi_finish()
{
	MPI_Finalize();
}
