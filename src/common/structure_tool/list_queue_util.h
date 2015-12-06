/*
 * list_queue_util.h
 *
 *  Created on: 2015年10月12日
 *      Author: ron
 */

#ifndef SRC_COMMON_STRUCTURE_TOOL_LIST_QUEUE_UTIL_H_
#define SRC_COMMON_STRUCTURE_TOOL_LIST_QUEUE_UTIL_H_

#include <stdio.h>
#include "basic_queue.h"
#include "basic_list.h"
#include "message.h"
#include "global.h"

void *list_to_array(list_t *list, uint64_t *size, uint64_t offset, uint64_t write_len);
basic_queue_t *list_to_queue(list_t *list, size_t size);

#endif /* SRC_COMMON_STRUCTURE_TOOL_LIST_QUEUE_UTIL_H_ */
