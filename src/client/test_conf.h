/*
 * test_conf.h
 *
 *  Created on: 2016年1月12日
 *      Author: liuming
 */

#ifndef SRC_CLIENT_TEST_CONF_H_
#define SRC_CLIENT_TEST_CONF_H_
#include "message.h"
#include "global.h"

#define EXECUTE_TIMES 3
#define FILE_NUM 3
#define BLOCK_NUM 14

#define NODE_PROCESS_NUM 3
#define MEMORY_SIZE SMALL

#define MULTI_SAME_FILE_NAME "multi_same_file"

#define MULTI_SAME_CLIENT_NUM 32
#define MULTI_SAME_TEST_FILE_SIZE (1UL << 24)

#define MULTI_DIFF_CLIENT_NUM 32
#define MULTI_DIFF_TEST_FILE_SIZE (1UL << 24)
#endif /* SRC_CLIENT_TEST_CONF_H_ */
