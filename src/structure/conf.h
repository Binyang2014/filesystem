#ifndef _FILEYSTEM_MAIN_CONF_H_
#define _FILEYSTEM_MAIN_CONF_H_
#include "mpi.h"

/**
 * 表示一个mpi环境中机器
 */
typedef struct mpi_machine{
	MPI_Comm comm;
	int rank;
}mpi_machine;

/**
 * information of master machine
 */
mpi_machine master;

/**
 * master MPI_COMM_WORLD id
 */

mpi_machine **backup_master;
#endif
