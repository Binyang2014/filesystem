/*
 * storage_t.h
 *
 *  Created on: 2015年10月8日
 *      Author: ron
 */

#ifndef SRC_DATA_MASTER_STORAGE_T_H_
#define SRC_DATA_MASTER_STORAGE_T_H_
#include "../common/structure_tool/sds.h"；

typedef struct {
	int rank;
	sds visual_ip;
	unsigned long used_blocks;
	unsigned long free_blocks;
}storage_machine_sta_t;

#endif /* SRC_DATA_MASTER_STORAGE_T_H_ */
