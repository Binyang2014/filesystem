/*
 * data_master.h
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 */

#ifndef SRC_DATA_MASTER_DATA_MASTER_H_
#define SRC_DATA_MASTER_DATA_MASTER_H_

#include "../common/structure_tool/name_space.h"

struct data_master{
	name_space_t *namespace;
};

typedef struct data_master data_master_t;

data_master_t* create_data_master(int machine_num);
void data_master_init(data_master_t *data_master);

#endif /* SRC_DATA_MASTER_DATA_MASTER_H_ */
