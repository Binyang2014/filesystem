#include <mpi.h>
#include "data_master.h"
#include "zmalloc.h"
#include "log.h"

int main(argc, argv)
	int argc;char ** argv; {
	int rank, size, provided;

	mpi_init_with_thread(&argc, &argv);
	size = get_mpi_size();
	rank = get_mpi_rank();


	if(rank == 0){

	}else{

	}
	mpi_finish();
	return 0;
}

