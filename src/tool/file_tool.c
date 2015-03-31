/*
 * file_tool.c
 *
 *  Created on: 2015年3月30日
 *      Author: ron
 */
#include "file_tool.h"
#include <stdio.h>

/**
 * 获取文件大小，如果文件不存在，返回-1
 */
long file_size(char *file_name){
	FILE *fp = fopen(file_name, "r");
	if(fp == NULL)
		return -1;
	fseek(fp, 0L, SEEK_END);
	return ftell(fp);
}


