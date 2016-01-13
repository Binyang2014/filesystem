/*
 * time_tool.h
 *
 *  Created on: 2016年1月5日
 *      Author: liuming
 */

#ifndef SRC_COMMON_STRUCTURE_TOOL_TIME_TOOL_H_
#define SRC_COMMON_STRUCTURE_TOOL_TIME_TOOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <inttypes.h>

typedef struct timeval timeval_t;
timeval_t * get_timestamp();
int cal_time(timeval_t *end, timeval_t *start);

#endif /* SRC_COMMON_STRUCTURE_TOOL_TIME_TOOL_H_ */
