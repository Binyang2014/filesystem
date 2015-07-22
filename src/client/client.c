/*
 * client.c
 *
 *  Created on: 2015年7月20日
 *      Author: ron
 */

#include "client.h"

static void create_tmp_file();

/*
 * client -> D_M file_name
 */
static void create_persistent_file();
static void read_tmp_file();
static void read_persistent_file();
static void delete_tmp_file();
static void delete_persistent_file();
static void consistent_persistent_file();

static void merge_file();
static void read_disk_file();
