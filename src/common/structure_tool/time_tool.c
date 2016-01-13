/*
 * time_tool.c
 *
 *  Created on: 2016年1月5日
 *      Author: liuming
 */
#include "time_tool.h"

timeval_t * get_timestamp()
{
	timeval_t *start = malloc(sizeof(*start));
	gettimeofday(start, NULL);
	return start;
}

int cal_time(timeval_t *end, timeval_t *start)
{
	return 1000000 * ( end->tv_sec - start->tv_sec ) + end->tv_usec - start->tv_usec;
}
