/*
 * master.h
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */

#ifndef SRC_MAIN_MASTER_H_
#define SRC_MAIN_MASTER_H_
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "conf.h"

void init_master_server_info(int);

void init_data_server_node();

request_node* malloc_request(char *buf, int size, MPI_Status *status);

/* master initialize
 */
master_init();

/**
 * there are p_threads in the master machine
 */

/**
 * receive information from client and data server
 * information can classify as two kinds
 * 1. client operation code
 * 2. data server heart blood information
 */
static void* master_server(void *arg);

static void* request_handler(void *arg);

static void log_backup();

static void heart_blood();

static void namespace_control();

static int create_file();

/*
 * data server
 */

static void data_server_init();

static data_servers *master_data_servers;

#endif /* SRC_MAIN_MASTER_H_ */
