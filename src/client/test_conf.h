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

#define NODE_PROCESS_NUM 10
#define MEMORY_SIZE LARGE

#define MULTI_SAME_FILE_NAME "multi_same_file"

#define MULTI_SAME_CLIENT_NUM (1 << 5)
#define MULTI_SAME_TEST_FILE_SIZE (1UL << 34)

#define MULTI_DIFF_CLIENT_NUM (1 << 5)
#define MULTI_DIFF_TEST_FILE_SIZE (1UL << 34)
#endif /* SRC_CLIENT_TEST_CONF_H_ */
