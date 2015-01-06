/**
 * 1. master
 * 2. dataserver
 * 3. 
 */
#include <stdio.h>
#include "mpi.h"
#include "conf.h"
#include "master.h"
#include "pthread.h"

int main(int argc, char *argv) 
{
	int rank, size;
	
	MPI_INIT(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(size < necessary_machine_num)
	{
		return MPI_Finalize();
	}
	if(rank == 0){
		master_init(0);
		return 0;
	}

	MPI_Finalize();
	return 0;
}
