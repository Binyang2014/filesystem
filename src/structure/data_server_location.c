/*
 * data_server_location.c
 *
 *  Created on: 2015年5月13日
 *      Author: ron
 */

#include <stdio.h>
#include <stdlib.h>
#include "data_server_location.h"

/**fun of file_machine_location**/
file_machine_location* create_file_machine_location(unsigned long count, int machine_id, int file_seq, int id_seq){
	file_machine_location *location = (file_machine_location *)malloc(sizeof(file_machine_location));
	if(location == NULL){
		return location;
	}

	location->count = count;
	location->file_seq_point.s = file_seq;
	location->file_seq_point.e = file_seq + count - 1;
	location->block_golobal_point.s = id_seq;
	location->block_golobal_point.e = id_seq + count - 1;
	location->machinde_id = machine_id;
	return location;
}

