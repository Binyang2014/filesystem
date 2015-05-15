/*
 * master.h
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */

#ifndef SRC_MAIN_MASTER_H_
#define SRC_MAIN_MASTER_H_

/* master initialize
 */
master_init();

/**
 * receive information from client and data server
 * information can classify as two kinds
 * 1. client operation code
 * 2. data server heart blood information
 */
static void* master_server(void *arg);

#endif /* SRC_MAIN_MASTER_H_ */
