#ifndef _FILEYSTEM_MAIN_CONF_H_
#define _FILEYSTEM_MAIN_CONF_H_
#include "mpi.h";

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
extern mpi_machine master;

const int necessary_machine_num = 3;
/**
 * master MPI_COMM_WORLD id
 */

mpi_machine **backup_master;
#endif
