/*
 * data_master.h
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 */

#ifndef SRC_DATA_MASTER_DATA_MASTER_H_
#define SRC_DATA_MASTER_DATA_MASTER_H_
#include <stddef.h>
#include "../common/structure_tool/name_space.h"
#include "../common/communication/message.h"

struct data_master{
	int rank;
	sds visual_ip;
	size_t group_size;		//size
	name_space_t *namespace;
	list_t *storage_machine_l;
	unsigned long free_size;
};

struct data_master_op{
	int (*machine_register)();
	void (*create_temp_file)(char *name);
	void (*append_temp_file)();
	void (*read_temp_file)();
	//unimplemented interface
	void (*delete_temp_file)(char *name);
};

typedef struct data_master data_master_t;

data_master_t* create_data_master(int machine_num);
void data_master_init(data_master_t *data_master);

#endif /* SRC_DATA_MASTER_DATA_MASTER_H_ */
