/*
 * master.h
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 */

#ifndef SRC_MASTER_MASTER_H_
#define SRC_MASTER_MASTER_H_

struct master {

};

typedef struct master master_t;

/**
 * size machine size
 */
master_t *create_master(size_t size);
void destroy_master(master_t *master);

#endif /* SRC_MASTER_MASTER_H_ */
