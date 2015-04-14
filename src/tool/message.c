/**
 * create on: 2015.3.29
 * author: Binyang
 *
 * This file offer complete some basic message functions
 */
#include "message.h"

int send_msg_read_ctod()
{
	return 0;
}

int recv_msg_read_ctod()
{
	return 0;
}

void printf_msg_status(MPI_Status* status)
{
	printf("The information comes from MPI status\n");
	printf("The MPI source is %d\n", status->MPI_SOURCE);
	printf("The MPI tag is %d\n", status->MPI_TAG);
	printf("The MPI error number is %d\n", status->MPI_ERROR);
}
