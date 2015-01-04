#ifndef _FILEYSTEM_MAIN_CONF_H_
#define _FILEYSTEM_MAIN_CONF_H_
#include "mpi.h";

/**
 * 表示一个mpi环境中机器
 */
struct mpi_machine{
	MPI_Comm
	int id;
};
/**
 * master MPI_COMM_WORLD id
 */

struct mpi_machine *master;

struct mpi_machine **backup_master;
#endif
