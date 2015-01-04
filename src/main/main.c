/**
 * 1. master
 * 2. dataserver
 * 3. 
 */
#include <stdio.h>
#include <mpi.h>

void init()
{
}

int main(int argc, char *argv) 
{
	int rank, size;
	
	MPI_INIT(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	return 0;
}
