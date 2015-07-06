/*
 * master.h
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */

#ifndef SRC_MASTER_MASTER_H_
#define SRC_MASTER_MASTER_H_

/* master initialize
 */
int master_init(int server_count);
int master_destroy();

/**
 * receive information from client and data server
 * information can classify as two kinds
 * 1. client operation code
 * 2. data server heart blood information
 */

#endif /* SRC_MASTER_MASTER_H_ */
