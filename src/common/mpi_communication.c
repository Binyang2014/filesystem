/*
 * mpi_communication.c
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 */
#include <stdio.h>
#include <assert.h>
#include "mpi_communication.h"

static void printf_msg_status(mpi_status_t* status)
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

