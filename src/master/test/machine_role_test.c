/*
 * machine_role_test.c
 *
 *  Created on: 2015年10月28日
 *      Author: ron
 */

#include <mpi.h>
#include <unistd.h>
#include "machine_role.h"
#include "zmalloc.h"
#include "mpi_communication.h"
#include "log.h"

int main(argc, argv)
	int argc;char ** argv; {
	int rank, size, provided;

	mpi_init_with_thread(&argc, &argv);
	rank = get_mpi_rank();
	size = get_mpi_size();

	map_role_value_t *role = NULL;
	char *net_name = "eth0";
	char *file_name = "topo.conf";
	log_init("", LOG_DEBUG);
	log_write(LOG_DEBUG, "file open successfully");
	if (rank == 0) {
		char *file_path = file_name;
		role = machine_role_allocator_start(size, rank, file_path, net_name);
		puts("server ******************************************************end");
	}else{

		usleep(50);
		role = get_role(rank, net_name);
		log_write(LOG_DEBUG, "get role successfully, ip = %s and rank = %d and type = %d", role->ip, role->rank, role->type);
		zfree(role);

		log_write(LOG_DEBUG, "file end successfully");
	}
	log_write(LOG_WARN, "SUCCESS!!!! role rank = %d, role ip = %s", role->rank, role->ip);
	mpi_finish();
	return 0;
}
