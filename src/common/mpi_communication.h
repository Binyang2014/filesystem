/*
 * mpi_communication.h
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 *      Modified: Binyang
 */

#ifndef SRC_COMMON_MPI_COMMUNICATION_H_
#define SRC_COMMON_MPI_COMMUNICATION_H_
#include <mpi.h>

struct mpi_status
{
	int source;
	int tag;
	int error_num;
	int size;
};
typedef struct mpi_status mpi_status_t;

void *mpi_send_recv(void* msg, int dst, int tag);
void *mpi_recv(void* msg, mpi_status_t status);


#endif /* SRC_COMMON_MPI_COMMUNICATION_H_ */
