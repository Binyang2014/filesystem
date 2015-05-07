/*
 * data_server_location.h
 *
 *  Created on: 2015年5月5日
 *      Author: ron
 */

#ifndef SRC_MAIN_DATA_SERVER_LOCATION_H_
#define SRC_MAIN_DATA_SERVER_LOCATION_H_

/**
 * file location information in one machine
 */
typedef struct file_machine_location{
	unsigned long count;		//file block count
	int machinde_id;   			//machine id
	struct file_machine_location *next;
	block file_block[256]; 		//file global block id
}file_machine_location;

/**
 * file location information
 */
typedef struct file_location_des{
	int machinde_count;
	file_machine_location *machines_head;
	file_machine_location *machines_tail;
}file_location_des;

/**fun of file_machine_location**/
file_machine_location* create_file_machine_location();

/*****fun of file_location_des*********/
file_location_des* create_file_location_des();


#endif /* SRC_MAIN_DATA_SERVER_LOCATION_H_ */
