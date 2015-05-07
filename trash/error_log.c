/*
 * error_log.c
 *
 *  Created on: 2015年4月7日
 *      Author: ron
 */
#include "error_log.h"

void print_mpi_status(MPI_Status *status){
	puts("===========MPI STATUS===========");
	printf("MPI_ERROR = %d\n", status->MPI_ERROR);
}

void log_debug(char *info){
	if(LOG_LEVEL < LOG_DEBUG){
		puts(info);
	}
}

void log_info(char *info){
	if(LOG_LEVEL < LOG_INFO){
		puts(info);
	}
}

void log_warn(char *info){
	if(LOG_LEVEL < LOG_WARN){
		puts(info);
	}
}

void log_error(char *info){
	if(LOG_LEVEL < LOG_ERROR){
		puts(info);
	}
}

void log_fatal(char *info){
	if(LOG_LEVEL < LOG_FATAL){
		puts(info);
	}
}



