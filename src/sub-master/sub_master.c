/*
 * sub_master.c
 *
 *  Created on: 2015年7月9日
 *      Author: ron
 */

#include "sub_master.h"

/*===============Private Declaration===============*/

static void read_temp_file();
//static void

/*---------------------Sub-Master RPC Service------------------*/
static void allocate_temp_file_space();
static void create_temp_file();
static void delete_temp_file();


/**
 * sub-master server
 *
 * receive data-master register information
 * receive data-master heart blood
 * create temporary file
 * read temporary file
 * delete temporary file
 * ask for allocate temporary file space
 * ask for allocate persistent file space
 */
static void sub_master_server() {

}

