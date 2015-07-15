/*
 * mpi_communication.h
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 *      Modified: Binyang
 *
 * In this file I want to simplify mpi communicate whith other mechine
 * I wrap basic mpi functions into another function with less args to finish
 * communitate task
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

void mpi_send(void* msg, int dst, int tag, int len);
void mpi_recv(void* msg, int source, int tag, int len, mpi_status_t* status_t);
void mpi_server_recv(void* msg, int len, mpi_status_t* status_t);
int get_mpi_size();
int get_mpi_rank();
void mpi_init_with_thread(int* argc, char ***argv);
void mpi_finish();

#endif /* SRC_COMMON_MPI_COMMUNICATION_H_ */
