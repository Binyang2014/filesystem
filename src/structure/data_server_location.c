/*
 * data_server_location.c
 *
 *  Created on: 2015年5月13日
 *      Author: ron
 */

#include <stdio.h>

static void add_location(file_location_des *location, file_machine_location * ){

}

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
}

/*****fun of file_location_des*********/
file_location_des* create_file_location_des(){
	file_location_des *location = (file_location_des *)malloc(sizeof(file_location_des));
	if(location == NULL){
		return location;
	}

	return location;
}
