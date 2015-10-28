/*
 * list_queue_util.h
 *
 *  Created on: 2015年10月12日
 *      Author: ron
 */

#ifndef SRC_COMMON_STRUCTURE_TOOL_LIST_QUEUE_UTIL_H_
#define SRC_COMMON_STRUCTURE_TOOL_LIST_QUEUE_UTIL_H_

#include "basic_queue.h"
#include "basic_list.h"
#include "../communication/message.h"

void *list_to_array(list_t *list, int size);

#endif /* SRC_COMMON_STRUCTURE_TOOL_LIST_QUEUE_UTIL_H_ */