/*
 * error_log.h
 *
 *  Created on: 2015年4月7日
 *      Author: ron
 */

#ifndef SRC_TOOL_ERROR_LOG_H_
#define SRC_TOOL_ERROR_LOG_H_
#include <mpi.h>
#include <stdio.h>
#include "../global.h"

extern void print_mpi_status(MPI_Status * status);

extern void log_debug(char *);

extern void log_info(char *);

extern void log_warn(char *);

extern void log_error(char *);

extern void log_fatal(char *);



#endif /* SRC_TOOL_ERROR_LOG_H_ */
